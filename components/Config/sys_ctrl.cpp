#include "sys_ctrl.h"
#include "file_storage.h"
#include "config.h"

uint8_t sys_ctrl::s_TimerCountInSecond;

const char sys_ctrl::s_Tag[] = "sys_ctrl";

void sys_ctrl::init()
{
    gpio_pad_select_gpio(BTN_CONFIG_PIN);
    ESP_ERROR_CHECK(gpio_set_direction(BTN_CONFIG_PIN, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(gpio_set_pull_mode(BTN_CONFIG_PIN, GPIO_PULLUP_ONLY));
    ESP_ERROR_CHECK(gpio_set_intr_type(BTN_CONFIG_PIN, GPIO_INTR_NEGEDGE));

    // Intterrupt number see below
    ESP_ERROR_CHECK(gpio_intr_enable(BTN_CONFIG_PIN));
    ESP_ERROR_CHECK(gpio_isr_register(handleInterruptGpio, NULL, ESP_INTR_FLAG_LOWMED, NULL));

    s_TimerCountInSecond = 0;
}

RUNNING_MODE_CONFIG sys_ctrl::getRunningMode()
{
    return Config::getInstance()->getBootMode();
}

void sys_ctrl::setRunningMode(RUNNING_MODE_CONFIG mode)
{
    Config *cfg = Config::getInstance();
    cfg->setBootMode(mode);
    void *pointer = &mode;
    cfg->writeCF("boot_mode", pointer, CF_DATA_TYPE_UINT8);
    delay_ms(500);
}

void sys_ctrl::showMessage(void* mesg)
{
    ESP_LOGD(s_Tag, "%s", (char*)mesg);
    vTaskDelete(NULL);
}

void sys_ctrl::gotoConfigMode(void* data)
{
    while(1)
    {
        if(TIME_PRESS_BTN_IN_SECOND <= s_TimerCountInSecond)
        {
            setRunningMode(SYSTEM_MODE_CONFIG);
            s_TimerCountInSecond = 0;
            delay_ms(1000);
            rebootSystem(0);
        }

        if(0 == GPIO_INPUT_GET(BTN_CONFIG_PIN))
        {
            ESP_LOGI(s_Tag, "Press button count time: %d", s_TimerCountInSecond);
            ++s_TimerCountInSecond;
            delay_ms(1000);
        }
        else
        {
            rebootSystem(0);
            break;
        }
    }
    vTaskDelete(NULL);
}

void sys_ctrl::handleInterruptGpio(void* arg)
{
    uint32_t gpio_num = 0;
    uint32_t gpio_intr_status = READ_PERI_REG(GPIO_STATUS_REG);   //read status to get interrupt status for GPIO0-31
    uint32_t gpio_intr_status_h = READ_PERI_REG(GPIO_STATUS1_REG);//read status1 to get interrupt status for GPIO32-39
    SET_PERI_REG_MASK(GPIO_STATUS_W1TC_REG, gpio_intr_status);    //Clear intr for gpio0-gpio31
    SET_PERI_REG_MASK(GPIO_STATUS1_W1TC_REG, gpio_intr_status_h); //Clear intr for gpio32-39
    do
    {
        if(gpio_num < 32)
        {
            if(gpio_intr_status & BIT(gpio_num))
            {
                //This is an isr handler, you should post an event to process it in RTOS queue.
                switch (gpio_num)
                {
                    case BTN_CONFIG_PIN:
                    {
                        s_TimerCountInSecond = 0;
                        xTaskCreate(gotoConfigMode, "gotoConfigMode", 4096, NULL, 1, NULL);
                        break;
                    }
                    default:
                    {
                        char mesg[128];
                        sprintf(mesg, "Dont handle this interrupt gpio %d", (int)gpio_num);
                        xTaskCreate(showMessage, "showinfo", 1024, (void *)mesg, tskIDLE_PRIORITY, NULL);
                        break;
                    }
                }
            }
        }
        else
        {
            if(gpio_intr_status_h & BIT(gpio_num - 32))
            {
                char mesg[128];
                sprintf(mesg, "Dont handle this interrupt gpio %d", (int)(gpio_num - 32));
                xTaskCreate(showMessage, "showinfo", 1024, (void *)mesg, tskIDLE_PRIORITY, NULL);
            }
        }
    } while(++gpio_num < GPIO_PIN_COUNT);
}

bool sys_ctrl::resetFactory()
{
    Config::getInstance()->restore();
    //profile::getInstance()->restore();
    return file_storage::factoryReset();
}