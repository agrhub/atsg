var CONTROLLER_NAME = {
	DEVICE_CMD_UNKNOW                   : 0,
    DEVICE_CMD_LAMP                     : 1,
    DEVICE_CMD_PUMP                     : 2,
    DEVICE_CMD_MISTING                  : 3,
    DEVICE_CMD_FAN                      : 4,
    DEVICE_CMD_AIR_CONDITIONER          : 5,
    DEVICE_CMD_CO2_CONTROLLER           : 6,
    DEVICE_CMD_DOSING_PUMP_CONTROLLER   : 7,
    DEVICE_CMD_OXYGEN_PUMP_CONTROLLER   : 8,
    DEVICE_CMD_VALVE_INPUT             	: 9,
    DEVICE_CMD_VALVE_OUTPUT            	: 10,
	DEVICE_CMD_HYDRO_TANK_WASHING 		: 11
};

var SENSOR_NAME = {
	LIGHT_SENSOR 	: 1,
	AIR_TEMP 		: 2,
	AIR_HUMIDITY 	: 3,
	SOIL_TEMP 		: 4,
	SOIL_HUMIDITY 	: 5,
	SOIL_EC 		: 6,
	SOIL_PH 		: 7,
	WATER_TEMP 		: 8,
	WATER_EC 		: 9,
	WATER_PH 		: 10,
	WATER_ORP 		: 11,
	BATTERY 		: 12,
	CO2 			: 13,
	WATER_LEVEL		: 14,
	WATER_DETECT	: 15,
	ERROR_DETECT	: 16
};

function Device(){
	var fn = this;
	this.devices = [];
	this.isBusy = false;

	this.init = function(){
		setInterval(this.loadDevice, 60000);
		this.loadDevice();
	};

	this.loadDevice = function(){
		if(!fn.isBusy){
			$.get("/api/v1/devices", function(data){
				fn.devices = data.devices;
				fn.initContent();
				fn.initEvent();
			});
		}
	};

	this.initSensorContent = function(sensor){
		var sensors_name = "";
		switch(sensor.device_name){
			case 4:
				sensors_name = LANG["dashboard"]["deviceName"]["soilSensor"];
				break;
			case 5:
			case 14:
			case 16:
				sensors_name = LANG["dashboard"]["deviceName"]["airSensor"];
				break;
			case 10:
				sensors_name = LANG["dashboard"]["deviceName"]["lightSensor"];
				break;
			case 17:
				sensors_name = LANG["dashboard"]["deviceName"]["floodSensor"];
				break;
			default:
				break;
		}

		var sensors = "";
		var sensor_battery = 0;
		var colors = ["red", "green", "blue", "orange", "pink", "purple", "grey"];
		var icons = {
				light : "icon-lightbulb-outline",
				temperature : "icon-thermometer",
				humidity : "icon-opacity",
				battery : "icon-battery",
				conductivity : "icon-opacity"
			};

		for(var i = 0;i < sensor.data.length;i++){
			var sensor_icon = "";
			var sensor_unit = "";
			var sensor_name = "";

			var sensor_value = sensor.data[i].sensor_value;
			switch(sensor.data[i].sensor_type){
				case SENSOR_NAME.LIGHT_SENSOR:
					sensor_icon = icons.light;
					sensor_unit = "lux";
					sensor_name = LANG["dashboard"]["sensorName"]["light"];
					break;
				case SENSOR_NAME.AIR_TEMP:
					sensor_icon = icons.temperature;
					sensor_unit = "*C";
					sensor_name = LANG["dashboard"]["sensorName"]["airTemp"];
					break;
				case SENSOR_NAME.AIR_HUMIDITY:
					sensor_icon = icons.humidity;
					sensor_unit = "%";
					sensor_name = LANG["dashboard"]["sensorName"]["airHumidity"];
					break;
				case SENSOR_NAME.SOIL_TEMP:
					sensor_icon = icons.temperature;
					sensor_unit = "*C";
					sensor_name = LANG["dashboard"]["sensorName"]["soilTemp"];
					break;
				case SENSOR_NAME.SOIL_HUMIDITY:
					sensor_icon = icons.humidity;
					sensor_unit = "%";
					sensor_name = LANG["dashboard"]["sensorName"]["soilHumidity"];
					break;
				case SENSOR_NAME.SOIL_EC:
					sensor_icon = icons.conductivity;
					sensor_unit = "PPM";
					sensor_name = LANG["dashboard"]["sensorName"]["soilEC"];
					break;
				case SENSOR_NAME.SOIL_PH:
					sensor_icon = icons.ph;
					sensor_unit = "PH";
					sensor_name = LANG["dashboard"]["sensorName"]["soilPH"];
					break;
				case SENSOR_NAME.WATER_TEMP:
					sensor_icon = icons.temperature;
					sensor_unit = "*C";
					sensor_name = LANG["dashboard"]["sensorName"]["waterTemp"];
					break;
				case SENSOR_NAME.WATER_EC:
					sensor_icon = icons.conductivity;
					sensor_unit = "mS/cm";
					sensor_name = LANG["dashboard"]["sensorName"]["waterEC"];
					break;
				case SENSOR_NAME.WATER_PH:
					sensor_icon = icons.ph;
					sensor_unit = "PH";
					sensor_name = LANG["dashboard"]["sensorName"]["waterPH"];
					break;
				case SENSOR_NAME.WATER_ORP:
					sensor_icon = icons.orp;
					sensor_unit = "mV";
					sensor_name = LANG["dashboard"]["sensorName"]["waterORP"];
					break;
				case SENSOR_NAME.CO2:
					sensor_icon = icons.humidity;
					sensor_unit = "PPM";
					sensor_name = LANG["dashboard"]["sensorName"]["co2"];
					break;
				case SENSOR_NAME.BATTERY:
					sensor_icon = icons.battery;
					sensor_unit = "%";
					sensor_name = LANG["dashboard"]["sensorName"]["bat"];
					sensor_battery = sensor_value;
					break;
				case SENSOR_NAME.WATER_DETECT:
					sensor_icon = icons.humidity;
					sensor_unit = "";
					sensor_name = LANG["dashboard"]["sensorName"]["waterLeak"];
					break;
				default:
					break;
			}
			if(sensor.data[i].sensor_type == SENSOR_NAME.BATTERY){
				continue;
			}

			sensors += '<div class="col s12 m6">' +
							'<div class="card-panel ' + colors[i] + ' white-text center m-b-0" style="padding: 10px">' +
								'<div class="row center m-b-0">' +
									'<p>' +
										'<i class="material-icons ' + sensor_icon + '" style="font-size: 20px"></i>' +
										'<span style="margin-top: 10px; font-size: 12px">' + sensor_name + '</span>	' +
									'</p>' +
								'</div>' +
								'<div class="row center m-b-0">' +
									'<span style="font-size: 25px">' + sensor_value + '</span>' +
									'<span style="font-size: 12px;line-height: 14px;margin-top: 5px;position: absolute;">'
										+ sensor_unit +
									'</span>' +
								'</div>' +
							'</div>' +
						'</div>';
		}

		var html = '<div class="col s12 m6 l4">' +
						'<div class="card white grey-text">' +
							'<div class="card-content">' +
								'<span class="card-title">' + sensors_name + '</span>' +
								'<span class="card-title" style="position:absolute;right:16px;top:16px"><i class="material-icons icon-battery" style="font-size: 20px"></i>' + sensor_battery + '%</span>' +
							'</div>' +
							'<div style="padding: 10px">' +
								'<div class="row">' +
									sensors +
								'</div>' +

							'</div>' +

						'</div>' +
					'</div>';

		return html;
	},

	this.initPowerContent = function(device){
		var device_name = "";
		switch(device.data[0].controller_type){
			case CONTROLLER_NAME.DEVICE_CMD_LAMP:
				device_name = LANG["dashboard"]["controllerName"]["lamp"];
				break;
			case CONTROLLER_NAME.DEVICE_CMD_PUMP:
				device_name = LANG["dashboard"]["controllerName"]["waterPump"];
				break;
			case CONTROLLER_NAME.DEVICE_CMD_MISTING:
				device_name = LANG["dashboard"]["controllerName"]["mistingPump"];
				break;
			case CONTROLLER_NAME.DEVICE_CMD_FAN:
				device_name = LANG["dashboard"]["controllerName"]["fan"];
				break;
			case CONTROLLER_NAME.DEVICE_CMD_AIR_CONDITIONER:
				device_name = LANG["dashboard"]["controllerName"]["ac"];
				break;
			case CONTROLLER_NAME.DEVICE_CMD_DOSING_PUMP_CONTROLLER:
				device_name = LANG["dashboard"]["controllerName"]["dosingPump"];
				break;
			case CONTROLLER_NAME.DEVICE_CMD_VALVE_INPUT:
				device_name = LANG["dashboard"]["controllerName"]["waterValveIn"];
				break;
			default:
				device_name = device.device_mac_address;
				break;
		}

		var device_state = '';
		if(device.data[0].controller_is_on){
			device_state = 'checked="checked"';
		}

		var html = "";

		if(device.data[0].controller_type == CONTROLLER_NAME.DEVICE_CMD_AIR_CONDITIONER){
			var air_conditioner_power_state = '';

			if(device.data[0].air_conditioner_power == "on"){
				air_conditioner_power_state = 'checked="checked"';
			}

			html =  '<div class="col s6 m3 l3">' +
						'<div class="card white grey-text" id="device_1">' +
							'<div class="card-content">' +
								'<span class="card-title">' + device_name + '</span>' +
							'</div>' +
							'<div style="padding: 10px" class="center">' +
								'<div class="switch">' +
									'<div class="row center m-b-0">' +
										'<label>' +
											'OFF' +
											'<input class="air_conditioner_state" id="ac_pwr" type="checkbox" ' + device_state + ' device-id="' + device.device_mac_address + '">' +
											'<span class="lever"></span>' +
											'ON' +
										'</label>' +
									'</div>'  +
								'</div></br>' +
								'<span class="card-title">' + LANG["dashboard"]["sensorName"]["airTemp"] + ' (*C)</span>' +
								'<div class="row">' +
									'<div class="col s4 m3 center">' +
										'<a class="btn-floating btn-small waves-effect waves-light btn_ac_temp" data-temp="16" device-id="' + device.device_mac_address + '"><span>16</span></a>' +
									'</div>' +
									'<div class="col s4 m3 center">' +
										'<a class="btn-floating btn-small waves-effect waves-light btn_ac_temp" data-temp="18" device-id="' + device.device_mac_address + '"><span>18</span></a>' +
									'</div>' +
									'<div class="col s4 m3 center">' +
										'<a class="btn-floating btn-small waves-effect waves-light btn_ac_temp" data-temp="20" device-id="' + device.device_mac_address + '"><span>20</span></a>' +
									'</div>' +
									'<div class="col s4 m3 center">' +
										'<a class="btn-floating btn-small waves-effect waves-light btn_ac_temp" data-temp="22" device-id="' + device.device_mac_address + '"><span>22</span></a>' +
									'</div>' +
								'</div>' +
								'<div class="row">' +
									'<div class="col s4 m3 center">' +
										'<a class="btn-floating btn-small waves-effect waves-light btn_ac_temp" data-temp="24" device-id="' + device.device_mac_address + '"><span>24</span></a>' +
									'</div>' +
									'<div class="col s4 m3 center">' +
										'<a class="btn-floating btn-small waves-effect waves-light btn_ac_temp" data-temp="26" device-id="' + device.device_mac_address + '"><span>26</span></a>' +
									'</div>' +
									'<div class="col s4 m3 center">' +
										'<a class="btn-floating btn-small waves-effect waves-light btn_ac_temp" data-temp="28" device-id="' + device.device_mac_address + '"><span>28</span></a>' +
									'</div>' +
									'<div class="col s4 m3 center">' +
										'<a class="btn-floating btn-small waves-effect waves-light btn_ac_temp" data-temp="30" device-id="' + device.device_mac_address + '"><span>30</span></a>' +
									'</div>' +
								'</div>' +
							'</div>' +
							'<div class="card-action center">' +
								'<div class="switch">' +
									'<div class="row center m-b-0">' +
										'<label>' +
											'RUNNING' +
											'<input class="air_conditioner_learning" id="ac_learning" type="checkbox" device-id="' + device.device_mac_address + '">' +
											'<span class="lever"></span>' +
											'LEARNING' +
										'</label>' +
									'</div>'  +
								'</div>' +
				            '</div>' +
						'</div>' +
					'</div>';
		}
		else{
			html =  '<div class="col s6 m3 l2">' +
						'<div class="card white grey-text" id="device_1">' +
							'<div class="card-content">' +
								'<span class="card-title">' + device_name + '</span>' +
							'</div>' +
							'<div style="padding: 10px" class="center">' +
								'<div class="switch">' +
									'<label>' +
										'OFF' +
										'<input class="power_switch" type="checkbox" ' + device_state + ' device-id="' + device.device_mac_address + '">' +
										'<span class="lever"></span>' +
										'ON' +
									'</label>' +
								'</div>' +
							'</div>' +
						'</div> ' +
					'</div>';
		}
		return html;
	},

	this.initTankControllerContent = function(device){
		var devices_name = LANG["dashboard"]["deviceName"]["smartTank"];
		var sensors = "";
		var controllers = "";
		var colors = ["red", "green", "blue", "orange", "pink", "purple", "grey", "light-green", "teal", "cyan", "blue-grey"];
		var icons = {
				light : "icon-lightbulb-outline",
				temperature : "icon-thermometer",
				humidity : "icon-opacity",
				battery : "icon-battery",
				conductivity : "icon-opacity"
			};

		for(var i = 0;i < device.data.length;i++){
			var sensor_type = device.data[i].sensor_type;
			var controller_type = device.data[i].controller_type;
			if(sensor_type != undefined){
				var sensor_icon = "";
				var sensor_unit = "";
				var sensor_name = "";

				var sensor_value = device.data[i].sensor_value;

				switch(sensor_type){
					case SENSOR_NAME.WATER_TEMP:
						sensor_icon = icons.temperature;
						sensor_unit = "*C";
						sensor_name = LANG["dashboard"]["sensorName"]["waterTemp"];
						break;
					case SENSOR_NAME.WATER_EC:
						sensor_icon = icons.conductivity;
						sensor_unit = "mS/cm";
						sensor_name = LANG["dashboard"]["sensorName"]["waterEC"];
						break;
					case SENSOR_NAME.WATER_PH:
						sensor_icon = icons.conductivity;
						sensor_unit = "PH";
						sensor_name = LANG["dashboard"]["sensorName"]["waterPH"];
						break;
					case SENSOR_NAME.WATER_ORP:
						sensor_icon = icons.conductivity;
						sensor_unit = "mV";
						sensor_name = LANG["dashboard"]["sensorName"]["waterORP"];
						break;
					case SENSOR_NAME.CO2:
						sensor_icon = icons.humidity;
						sensor_unit = "PPM";
						sensor_name = LANG["dashboard"]["sensorName"]["co2"];
						break;
					case SENSOR_NAME.WATER_LEVEL:
						sensor_icon = icons.conductivity;
						sensor_unit = "%";
						sensor_name = LANG["dashboard"]["sensorName"]["waterLevel"];
						break;
					case SENSOR_NAME.WATER_DETECT:
						sensor_icon = icons.humidity;
						sensor_unit = "";
						sensor_name = LANG["dashboard"]["sensorName"]["waterLeak"];
						break;
					case SENSOR_NAME.ERROR_DETECT:
						sensor_icon = icons.conductivity;
						sensor_unit = "";
						sensor_name = LANG["dashboard"]["sensorName"]["error"];
						break;
					default:
						break;
				}

				sensors += '<div class="col s12 m6 l4">' +
								'<div class="card-panel ' + colors[i] + ' white-text center m-b-0" style="padding: 10px">' +
									'<div class="row center m-b-0">' +
										'<p>' +
											'<i class="material-icons ' + sensor_icon + '" style="font-size: 20px"></i>' +
											'<span style="margin-top: 10px; font-size: 12px">' + sensor_name + '</span>	' +
										'</p>' +
									'</div>' +
									'<div class="row center m-b-0">' +
										'<span style="font-size: 25px">' + sensor_value + '</span>' +
										'<span style="font-size: 12px;line-height: 14px;margin-top: 5px;position: absolute;">'
											+ sensor_unit+
										'</span>' +
									'</div>' +
								'</div>' +
							'</div>';
			}
			else{
				

				var device_name = "";
				switch(controller_type){
					case CONTROLLER_NAME.DEVICE_CMD_PUMP:
						device_name = LANG["dashboard"]["controllerName"]["waterPump"];
						break;
					case CONTROLLER_NAME.DEVICE_CMD_OXYGEN_PUMP_CONTROLLER:
						device_name = LANG["dashboard"]["controllerName"]["oxygenPump"];
						break;
					case CONTROLLER_NAME.DEVICE_CMD_VALVE_INPUT:
						device_name = LANG["dashboard"]["controllerName"]["waterValveIn"];
						break;
					case CONTROLLER_NAME.DEVICE_CMD_VALVE_OUTPUT:
						device_name = LANG["dashboard"]["controllerName"]["waterValveOut"];
						break;
					case CONTROLLER_NAME.DEVICE_CMD_HYDRO_TANK_WASHING:
						device_name = LANG["dashboard"]["controllerName"]["washingTank"];
						break;
					default:
						break;
				}

				var device_state = '';
				if(device.data[i].controller_is_on){
					device_state = 'checked="checked"';
				}

				controllers += '<div class="col s12 m6 l4">' +
								'<div class="card-panel ' + colors[i] + ' white-text center m-b-0" style="padding: 10px">' +
									'<div class="row center m-b-0">' +
										'<p>' +
											'<span style="margin-top: 10px; font-size: 12px">' + device_name + '</span>	' +
										'</p>' +
									'</div>' +
									'<div class="row center m-b-0">' +
										'<div class="switch">' +
											'<label style="color:#FFFFFF">' +
												'TẮT' +
												'<input type="checkbox" ' + device_state + ' device-id="' + device.device_mac_address + '" controller-type="' + controller_type + '">' +
												'<span class="lever"></span>' +
												'BẬT' +
											'</label>' +
										'</div>' +
									'</div>' +
								'</div>' +
							'</div>';
			}

		}

		var html = '<div class="col s12 m6">' +
						'<div class="card white grey-text">' +
							'<div class="card-content">' +
								'<span class="card-title activator">' + devices_name + '<i class="material-icons right btn-open-settings icon-settings"></i></span>' +
							'</div>' +
							'<div style="padding: 10px">' +
								'<div class="row">' +
									sensors +
								'</div>' +
								'<div class="row">' +
									controllers +
								'</div>' +

							'</div>' +
							'<div class="card-reveal">' +
						      	'<span class="card-title grey-text text-darken-4">' + LANG["menu"]["setting"] + '<i class="material-icons right btn-close-settings icon-close"></i></span>' +
						      	'<div class="row">' +
						      		'<form class="col s12">' +
						      			'<label>EC</label>' +
						      			'<div class="row">' +
						      				'<div class="input-field col s6">' +
									          	'<input placeholder="EC min" id="ec_min" type="number" class="validate">' +
									          	//'<label for="ec_min">EC MIN</label>' +
									        '</div>' +
									        '<div class="input-field col s6">' +
									          	'<input placeholder="EC max" id="ec_max" type="number" class="validate">' +
									          	//'<label for="ec_max">EC MAX</label>' +
									        '</div>' +
						      			'</div>' +
						      			'<div class="row">' +
						      				'<div class="input-field col s4">' +
									          	'<input placeholder="A(%)" id="ec_fomular_a" type="number" class="validate">' +
									          	//'<label for="ec_fomular_a">A(%)</label>' +
									        '</div>' +
									        '<div class="input-field col s4">' +
									          	'<input placeholder="B(%)" id="ec_fomular_b" type="number" class="validate">' +
									          	//'<label for="ec_fomular_b">B(%)</label>' +
									        '</div>' +
									        '<div class="input-field col s4">' +
									          	'<input placeholder="C(%)" id="ec_fomular_c" type="number" class="validate">' +
									          	//'<label for="ec_fomular_c">C(%)</label>' +
									        '</div>' +
									        '<div class="input-field col s4">' +
									          	'<input placeholder="D(%)" id="ec_fomular_d" type="number" class="validate">' +
									          	//'<label for="ec_fomular_c">C(%)</label>' +
									        '</div>' +
						      			'</div>' +
						      			'<label>pH</label>' +
						      			'<div class="row">' +
						      				'<div class="input-field col s6">' +
									          	'<input placeholder="pH min" id="ph_min" type="number" class="validate">' +
									          	//'<label for="ph_min">PH MIN</label>' +
									        '</div>' +
									        '<div class="input-field col s6">' +
									          	'<input placeholder="pH max" id="ph_max" type="number" class="validate">' +
									          	//'<label for="ph_max">PH MAX</label>' +
									        '</div>' +
						      			'</div>' +
						      			'<label>Water pump frequence</label>' +
						      			'<div class="row">' +
						      				'<div class="input-field col s6">' +
									          	'<input placeholder="Minutes" id="water_pump_on_freq" type="number" class="validate">' +
									          	//'<label for="water_pump_on_freq">ON Time</label>' +
									        '</div>' +
									        '<div class="input-field col s6">' +
									          	'<input placeholder="Minutes" id="water_pump_off_freq" type="number" class="validate">' +
									          	//'<label for="water_pump_off_freq">OFF Time</label>' +
									        '</div>' +
						      			'</div>' +
						      			'<label>Water level</label>' +
						      			'<div class="row">' +
						      				'<div class="input-field col s6">' +
									          	'<input placeholder="Water level min (%)" id="water_level_min" type="number" class="validate">' +
									        '</div>' +
									        '<div class="input-field col s6">' +
									          	'<input placeholder="Water level max (%)" id="water_level_max" type="number" class="validate">' +
									        '</div>' +
						      				'<div class="input-field col s6">' +
									          	'<input placeholder="Tank height (CM)" id="tank_height" type="number" class="validate">' +
									        '</div>' +
									        '<div class="input-field col s6">' +
									          	'<input placeholder="Sensor height (CM)" id="sensor_height" type="number" class="validate">' +
									        '</div>' +
						      			'</div>' +
						      			'<div class="row">' +
						      				'<a href="#!" class="modal-close waves-effect waves-green btn btn-commit center" style="display:block">' + LANG["information"]["cmdUpdate"] + '</a>' +
						      			'<div>' +
						      		'</form>' +
						      	'</div>' +
						    '</div>' +
						'</div>' +
					'</div>';

		return html;
	},

	this.initContent = function(){
		var fn = this;
		var sensors = $('<div class="row"></div>');
		var powers = $('<div class="row"></div>');
		
		var ac_temp = 0;

		for(var i = 0; i < fn.devices.length; i++){
			var device = fn.devices[i];
			if(device.device_type == 1){
				sensors.append(fn.initSensorContent(device));
			}
			else if(device.device_type == 2){
				if(device.data.length > 0){
					powers.append(fn.initPowerContent(device));
					if(device.data[0].controller_type == 5){
						ac_temp = device.data[0].air_conditioner_temp;
					}
				}
			}
			else if(device.device_type == 3){
				//tank controller
				powers.append(fn.initTankControllerContent(device));
			}
		}

		$("#sensor-container").html(sensors);
		$("#device-container").html(powers);
		powers.find(".btn_ac_temp[data-temp='" + ac_temp + "']").addClass("pulse");
		
		$("#device-container .btn-open-settings").click(function(){
			fn.isBusy = true;
			$.get("/api/v1/setting", function(settings){
				//settings = JSON.parse(settings);
				//bind setting
				for(var i = 0; i < settings.length; i++){
					var setting = settings[i];
					if(setting.sensor_type != undefined && setting.sensor_type == SENSOR_NAME.WATER_EC){
						$('#ec_min').val(setting.sensor_threshold.min);
						$('#ec_max').val(setting.sensor_threshold.max);
						$('#ec_fomular_a').val(setting.sensor_formula.a);
						$('#ec_fomular_b').val(setting.sensor_formula.b);
						$('#ec_fomular_c').val(setting.sensor_formula.c);
						$('#ec_fomular_d').val(setting.sensor_formula.d);
					}
					else if(setting.sensor_type != undefined && setting.sensor_type == SENSOR_NAME.WATER_PH){
						$('#ph_min').val(setting.sensor_threshold.min);
						$('#ph_max').val(setting.sensor_threshold.max);
					}
					else if(setting.sensor_type != undefined && setting.sensor_type == SENSOR_NAME.WATER_LEVEL){
						$('#water_level_min').val(setting.sensor_threshold.min);
						$('#water_level_max').val(setting.sensor_threshold.max);
						$('#tank_height').val(setting.tank_height);
						$('#sensor_height').val(setting.sensor_height);
					}
					else if(setting.controller_type == CONTROLLER_NAME.DEVICE_CMD_PUMP){
						$('#water_pump_on_freq').val(setting.controller_freq.on);
						$('#water_pump_off_freq').val(setting.controller_freq.off);
					}
				}
				
			});
		});

		$("#device-container .btn-close-settings").click(function(){
			fn.isBusy = false;
		});

		$("#device-container .btn-commit").click(function(){
			var params = {
				ec_min : $("#ec_min").val(),
				ec_max : $("#ec_max").val(),
				ec_fomular_a : $("#ec_fomular_a").val(),
				ec_fomular_b : $("#ec_fomular_b").val(),
				ec_fomular_c : $("#ec_fomular_c").val(),
				ec_fomular_d : $("#ec_fomular_d").val(),
				ph_min : $("#ph_min").val(),
				ph_max : $("#ph_max").val(),
				water_pump_on_freq : $("#water_pump_on_freq").val(),
				water_pump_off_freq : $("#water_pump_off_freq").val(),
				water_level_min : $("#water_level_min").val(),
				water_level_max : $("#water_level_max").val(),
				tank_height : $("#tank_height").val(),
				sensor_height : $("#sensor_height").val(),
			};

			$.post("/api/v1/setting", params, function(settings){
				//bind setting
				for(var i = 0; i < settings.length; i++){
					var setting = settings[i];
					if(setting.sensor_type != undefined && setting.sensor_type == SENSOR_NAME.WATER_EC){
						$('#ec_min').val(setting.sensor_threshold.min);
						$('#ec_max').val(setting.sensor_threshold.max);
						$('#ec_fomular_a').val(setting.sensor_formula.a);
						$('#ec_fomular_b').val(setting.sensor_formula.b);
						$('#ec_fomular_c').val(setting.sensor_formula.c);
						$('#ec_fomular_d').val(setting.sensor_formula.d);
					}
					else if(setting.sensor_type != undefined && setting.sensor_type == SENSOR_NAME.WATER_PH){
						$('#ph_min').val(setting.sensor_threshold.min);
						$('#ph_max').val(setting.sensor_threshold.max);
					}
					else if(setting.sensor_type != undefined && setting.sensor_type == SENSOR_NAME.WATER_LEVEL){
						$('#water_level_min').val(setting.sensor_threshold.min);
						$('#water_level_max').val(setting.sensor_threshold.max);
						$('#tank_height').val(setting.tank_height);
						$('#sensor_height').val(setting.sensor_height);
					}
					else if(setting.controller_type == CONTROLLER_NAME.DEVICE_CMD_PUMP){
						$('#water_pump_on_freq').val(setting.controller_freq.on);
						$('#water_pump_off_freq').val(setting.controller_freq.off);
					}
				}
				
			});

			$("#device-container .btn-close-settings").trigger("click");
		});
	};

	this.initEvent = function(){
		$("#device-container input:checkbox").change(function() {
			fn.isBusy = true;
			var state = false;
			var checkbox = $(this);
			if(checkbox.is(':checked')){
			   state = true;
			}

			var device_id = $(this).attr("device-id");
			var controller_type = $(this).attr("controller-type");
			if(checkbox.hasClass("air_conditioner_learning")){
				return;
			}

			var isLearningMode = false;

			if(checkbox.hasClass("air_conditioner_state")){
				isLearningMode = $("#ac_learning").is(":checked");
				if(isLearningMode){
					var params = {
						device_id :device_id,
						cmd : (state ? "cmd_on" : "cmd_off")
					};

					$.post("/api/v1/ir_learning", params, function(result){
						fn.isBusy = false;
						var $toastContent = "";
						if(result["success"]){
							$toastContent = $('<span class="">' + LANG["dashboard"]["irLearningSuccessfully"] + '</span>');
						}
						else{
							$toastContent = $('<span class="">' + LANG["dashboard"]["irLearningFailed"] + '</span>');	
						}
						
  						Materialize.toast($toastContent, 5000);
					});
				}
			}
			if(!isLearningMode){
				var params = {
					device_id :device_id,
					device_state : state,
					controller_type : controller_type
				};

				$.post("/api/v1/device", params, function(device){
					fn.isBusy = false;
					for(var i = 0; i< fn.devices.length;i++){
						if(fn.devices[i].device_mac_address === device_id){
							fn.devices[i] = device;
							break;
						}
					}
					checkbox.attr("checked", "checked");
				});
			}
		});

		$("#device-container .btn_ac_temp").on("click", function(){
			$("#device-container .btn_ac_temp").removeClass("pulse");
			fn.isBusy = true;
			var btn = $(this).addClass("pulse");
			var device_id = btn.attr("device-id");
			var ac_temp = "cmd_temp_" + $(this).attr("data-temp");
			var isLearningMode = $("#ac_learning").is(":checked");
			if(isLearningMode){
				var params = {
					device_id :device_id,
					cmd : ac_temp
				};

				$.post("/api/v1/ir_learning", params, function(result){
					fn.isBusy = false;
					var $toastContent = "";
					if(result["success"]){
						$toastContent = $('<span class="">' + LANG["dashboard"]["irLearningSuccessfully"] + '</span>');
					}
					else{
						$toastContent = $('<span class="">' + LANG["dashboard"]["irLearningFailed"] + '</span>');	
					}

					btn.removeClass("pulse");
					
					Materialize.toast($toastContent, 5000);
				});
			}
			else{
				var params = {
					device_id :device_id,
					device_state : true,
					controller_mode : ac_temp
				};
				$.post("/api/v1/device", params, function(device){
					fn.isBusy = false;
				});
			}
		});
	};
};

