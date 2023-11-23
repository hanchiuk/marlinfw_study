// MarlinCore.cpp

#define __AVR__
#define ARDUINO

#include "MarlinCore.h"
#include "HAL/shared/Delay.h"
#include "HAL/shared/esp_wifi.h"
#include "HAL/shared/cpu_exception/exception_hook.h"

#if ENABLED(WIFISUPPORT)
#include "HAL/shared/esp_wifi.h"
#endif

#ifdef ARDUINO
#include "arduino-core/pins_arduino.h"
#endif

#include <math.h>

/*
#include "module/endstops.h"
#include "module/motion.h"
#include "module/planner.h"
#include "module/printcounter.h" // PrintCounter or Stopwatch
#include "module/settings.h"
#include "module/stepper.h"
#include "module/temperature.h"

#include "gcode/gcode.h"
#include "gcode/parser.h"
#include "gcode/queue.h"

#include "feature/pause.h"
#include "sd/cardreader.h"

#include "lcd/marlinui.h"
*/
// selective includes - start
#if 1
#if HAS_TOUCH_BUTTONS
#include "lcd/touch/touch_buttons.h"
#endif

#if HAS_TFT_LVGL_UI
#include "lcd/extui/mks_ui/tft_lvgl_configuration.h"
#include "lcd/extui/mks_ui/draw_ui.h"
#include "lcd/extui/mks_ui/mks_hardware.h"
#include <lvgl.h>
#endif

#if HAS_DWIN_E3V2
#include "lcd/e3v2/common/encoder.h"
#if ENABLED(DWIN_CREALITY_LCD)
#include "lcd/e3v2/creality/dwin.h"
#elif ENABLED(DWIN_LCD_PROUI)
#include "lcd/e3v2/proui/dwin.h"
#elif ENABLED(DWIN_CREALITY_LCD_JYERSUI)
#include "lcd/e3v2/jyersui/dwin.h"
#endif
#endif

#if HAS_ETHERNET
#include "feature/ethernet.h"
#endif

#if ENABLED(IIC_BL24CXX_EEPROM)
#include "libs/BL24CXX.h"
#endif

#if ENABLED(DIRECT_STEPPING)
#include "feature/direct_stepping.h"
#endif

#if ENABLED(HOST_ACTION_COMMANDS)
#include "feature/host_actions.h"
#endif

#if HAS_BEEPER
#include "libs/buzzer.h"
#endif

#if ENABLED(EXTERNAL_CLOSED_LOOP_CONTROLLER)
#include "feature/closedloop.h"
#endif

#if HAS_MOTOR_CURRENT_I2C
#include "feature/digipot/digipot.h"
#endif

#if ENABLED(MIXING_EXTRUDER)
#include "feature/mixing.h"
#endif

#if ENABLED(MAX7219_DEBUG)
#include "feature/max7219.h"
#endif

#if HAS_COLOR_LEDS
#include "feature/leds/leds.h"
#endif

#if ENABLED(BLTOUCH)
#include "feature/bltouch.h"
#endif

#if ENABLED(BD_SENSOR)
#include "feature/bedlevel/bdl/bdl.h"
#endif

#if ENABLED(POLL_JOG)
#include "feature/joystick.h"
#endif

#if HAS_SERVOS
#include "module/servo.h"
#endif

#if HAS_MOTOR_CURRENT_DAC
#include "feature/dac/stepper_dac.h"
#endif

#if ENABLED(EXPERIMENTAL_I2CBUS)
#include "feature/twibus.h"
#endif

#if ENABLED(I2C_POSITION_ENCODERS)
#include "feature/encoder_i2c.h"
#endif

#if (HAS_TRINAMIC_CONFIG || HAS_TMC_SPI) && DISABLED(PSU_DEFAULT_OFF)
#include "feature/tmc_util.h"
#endif

#if HAS_CUTTER
#include "feature/spindle_laser.h"
#endif

#if ENABLED(SDSUPPORT)
CardReader card;
#endif

#if ENABLED(G38_PROBE_TARGET)
uint8_t G38_move; // = 0
bool G38_did_trigger; // = false
#endif

#if ENABLED(DELTA)
#include "module/delta.h"
#elif ENABLED(POLARGRAPH)
#include "module/polargraph.h"
#elif IS_SCARA
#include "module/scara.h"
#endif

#if HAS_LEVELING
#include "feature/bedlevel/bedlevel.h"
#endif

#if ENABLED(GCODE_REPEAT_MARKERS)
#include "feature/repeat.h"
#endif

#if ENABLED(POWER_LOSS_RECOVERY)
#include "feature/powerloss.h"
#endif

#if ENABLED(CANCEL_OBJECTS)
#include "feature/cancel_object.h"
#endif

#if HAS_FILAMENT_SENSOR
#include "feature/runout.h"
#endif

#if EITHER(PROBE_TARE, HAS_Z_SERVO_PROBE)
#include "module/probe.h"
#endif

#if ENABLED(HOTEND_IDLE_TIMEOUT)
#include "feature/hotend_idle.h"
#endif

#if ENABLED(TEMP_STAT_LEDS)
#include "feature/leds/tempstat.h"
#endif

#if ENABLED(CASE_LIGHT_ENABLE)
#include "feature/caselight.h"
#endif

#if HAS_FANMUX
#include "feature/fanmux.h"
#endif

//#include "module/tool_change.h"

#if HAS_FANCHECK
#include "feature/fancheck.h"
#endif

#if ENABLED(USE_CONTROLLER_FAN)
#include "feature/controllerfan.h"
#endif

#if HAS_PRUSA_MMU1
#include "feature/mmu/mmu.h"
#endif

#if HAS_PRUSA_MMU2
#include "feature/mmu/mmu2.h"
#endif

#if ENABLED(PASSWORD_FEATURE)
#include "feature/password/password.h"
#endif

#if ENABLED(DGUS_LCD_UI_MKS)
#include "lcd/extui/dgus/DGUSScreenHandler.h"
#endif

#if HAS_DRIVER_SAFE_POWER_PROTECT
#include "feature/stepper_driver_safety.h"
#endif

#if ENABLED(PSU_CONTROL)
#include "feature/power.h"
#endif

#if ENABLED(EASYTHREED_UI)
#include "feature/easythreed_ui.h"
#endif

#if ENABLED(MARLIN_TEST_BUILD)
#include "tests/marlin_tests.h"
#endif

#endif
// selective includes - end

//PGMSTR(M112_KILL_STR, "M112 Shutdown");
MarlinState marlin_state = MF_INITIALIZING;

// For M109 and M190, this flag may be cleared (by M108) to exit the wait loop
bool wait_for_heatup = true;

// For M0/M1, this flag may be cleared (by M108) to exit the wait-for-user loop
#if HAS_RESUME_CONTINUE
bool wait_for_user; // = false;
void wait_for_user_response(millis_t ms/*=0*/, const bool no_sleep/*=false*/) {
	/*
	UNUSED(no_sleep);
	KEEPALIVE_STATE(PAUSED_FOR_USER);
	wait_for_user = true;
	if (ms) ms += millis(); // expire time
	while (wait_for_user && !(ms && ELAPSED(millis(), ms)))
		idle(TERN_(ADVANCED_PAUSE_FEATURE, no_sleep));
	wait_for_user = false;
	while (ui.button_pressed()) safe_delay(50);
	*/
}
#endif


inline void tmc_standby_setup() {
#if PIN_EXISTS(X_STDBY)
	SET_INPUT_PULLDOWN(X_STDBY_PIN);
#endif
#if PIN_EXISTS(X2_STDBY)
	SET_INPUT_PULLDOWN(X2_STDBY_PIN);
#endif
#if PIN_EXISTS(Y_STDBY)
	SET_INPUT_PULLDOWN(Y_STDBY_PIN);
#endif
#if PIN_EXISTS(Y2_STDBY)
	SET_INPUT_PULLDOWN(Y2_STDBY_PIN);
#endif
#if PIN_EXISTS(Z_STDBY)
	SET_INPUT_PULLDOWN(Z_STDBY_PIN);
#endif
#if PIN_EXISTS(Z2_STDBY)
	SET_INPUT_PULLDOWN(Z2_STDBY_PIN);
#endif
#if PIN_EXISTS(Z3_STDBY)
	SET_INPUT_PULLDOWN(Z3_STDBY_PIN);
#endif
#if PIN_EXISTS(Z4_STDBY)
	SET_INPUT_PULLDOWN(Z4_STDBY_PIN);
#endif
#if PIN_EXISTS(I_STDBY)
	SET_INPUT_PULLDOWN(I_STDBY_PIN);
#endif
#if PIN_EXISTS(J_STDBY)
	SET_INPUT_PULLDOWN(J_STDBY_PIN);
#endif
#if PIN_EXISTS(K_STDBY)
	SET_INPUT_PULLDOWN(K_STDBY_PIN);
#endif
#if PIN_EXISTS(U_STDBY)
	SET_INPUT_PULLDOWN(U_STDBY_PIN);
#endif
#if PIN_EXISTS(V_STDBY)
	SET_INPUT_PULLDOWN(V_STDBY_PIN);
#endif
#if PIN_EXISTS(W_STDBY)
	SET_INPUT_PULLDOWN(W_STDBY_PIN);
#endif
#if PIN_EXISTS(E0_STDBY)
	SET_INPUT_PULLDOWN(E0_STDBY_PIN);
#endif
#if PIN_EXISTS(E1_STDBY)
	SET_INPUT_PULLDOWN(E1_STDBY_PIN);
#endif
#if PIN_EXISTS(E2_STDBY)
	SET_INPUT_PULLDOWN(E2_STDBY_PIN);
#endif
#if PIN_EXISTS(E3_STDBY)
	SET_INPUT_PULLDOWN(E3_STDBY_PIN);
#endif
#if PIN_EXISTS(E4_STDBY)
	SET_INPUT_PULLDOWN(E4_STDBY_PIN);
#endif
#if PIN_EXISTS(E5_STDBY)
	SET_INPUT_PULLDOWN(E5_STDBY_PIN);
#endif
#if PIN_EXISTS(E6_STDBY)
	SET_INPUT_PULLDOWN(E6_STDBY_PIN);
#endif
#if PIN_EXISTS(E7_STDBY)
	SET_INPUT_PULLDOWN(E7_STDBY_PIN);
#endif
}

void setup(void) {
#ifdef FASTIO_INIT
	FASTIO_INIT();
#endif
#ifdef BOARD_PREINIT
	BOARD_PREINIT(); // Low-level init (before serial init)
#endif
	tmc_standby_setup();  // TMC Low Power Standby pins must be set early or they're not usable

	// Check startup - does nothing if bootloader sets MCUSR to 0
	//const byte mcu = hal.get_reset_source();
	//hal.clear_reset_source();

#if ENABLED(MARLIN_DEV_MODE)
	auto log_current_ms = [&](PGM_P const msg) {
		SERIAL_ECHO_START();
		SERIAL_CHAR('['); SERIAL_ECHO(millis()); SERIAL_ECHOPGM("] ");
		SERIAL_ECHOLNPGM_P(msg);
	};
#define SETUP_LOG(M) log_current_ms(PSTR(M))
#else
#define SETUP_LOG(...) NOOP
#endif
#define SETUP_RUN(C) do{ SETUP_LOG(STRINGIFY(C)); C; }while(0)

	//MYSERIAL1.begin(BAUDRATE);
	millis_t serial_connect_timeout = millis() + 1000UL;
	//while (!MYSERIAL1.connected() && PENDING(millis(), serial_connect_timeout)) { //nada }

#if HAS_MULTI_SERIAL && !HAS_ETHERNET
#ifndef BAUDRATE_2
#define BAUDRATE_2 BAUDRATE
#endif
	MYSERIAL2.begin(BAUDRATE_2);
	serial_connect_timeout = millis() + 1000UL;
	while (!MYSERIAL2.connected() && PENDING(millis(), serial_connect_timeout)) { //nada }
#ifdef SERIAL_PORT_3
#ifndef BAUDRATE_3
#define BAUDRATE_3 BAUDRATE
#endif
	MYSERIAL3.begin(BAUDRATE_3);
	serial_connect_timeout = millis() + 1000UL;
	while (!MYSERIAL3.connected() && PENDING(millis(), serial_connect_timeout)) { //nada }
#endif
#endif
	//SERIAL_ECHOLNPGM("start");

	// Set up these pins early to prevent suicide
#if HAS_KILL
	SETUP_LOG("KILL_PIN");
#if KILL_PIN_STATE
	SET_INPUT_PULLDOWN(KILL_PIN);
#else
	SET_INPUT_PULLUP(KILL_PIN);
#endif
#endif

#if ENABLED(FREEZE_FEATURE)
	SETUP_LOG("FREEZE_PIN");
#if FREEZE_STATE
	SET_INPUT_PULLDOWN(FREEZE_PIN);
#else
	SET_INPUT_PULLUP(FREEZE_PIN);
#endif
#endif

#if HAS_SUICIDE
	SETUP_LOG("SUICIDE_PIN");
	OUT_WRITE(SUICIDE_PIN, !SUICIDE_PIN_STATE);
#endif

#ifdef JTAGSWD_RESET
	SETUP_LOG("JTAGSWD_RESET");
	JTAGSWD_RESET();
#endif

	// Disable any hardware debug to free up pins for IO
#if ENABLED(DISABLE_DEBUG) && defined(JTAGSWD_DISABLE)
	delay(10);
	SETUP_LOG("JTAGSWD_DISABLE");
	JTAGSWD_DISABLE();
#elif ENABLED(DISABLE_JTAG) && defined(JTAG_DISABLE)
	delay(10);
	SETUP_LOG("JTAG_DISABLE");
	JTAG_DISABLE();
#endif

	TERN_(DYNAMIC_VECTORTABLE, hook_cpu_exceptions()); // If supported, install Marlin exception handlers at runtime

	SETUP_RUN(hal.init());

	// Init and disable SPI thermocouples; this is still needed
#if TEMP_SENSOR_IS_MAX_TC(0) || (TEMP_SENSOR_IS_MAX_TC(REDUNDANT) && REDUNDANT_TEMP_MATCH(SOURCE, E0))
	OUT_WRITE(TEMP_0_CS_PIN, HIGH);  // Disable
#endif
#if TEMP_SENSOR_IS_MAX_TC(1) || (TEMP_SENSOR_IS_MAX_TC(REDUNDANT) && REDUNDANT_TEMP_MATCH(SOURCE, E1))
	OUT_WRITE(TEMP_1_CS_PIN, HIGH);
#endif

#if ENABLED(DUET_SMART_EFFECTOR) && PIN_EXISTS(SMART_EFFECTOR_MOD)
	OUT_WRITE(SMART_EFFECTOR_MOD_PIN, LOW);   // Put Smart Effector into NORMAL mode
#endif

#if HAS_FILAMENT_SENSOR
	SETUP_RUN(runout.setup());
#endif

#if HAS_TMC220x
	SETUP_RUN(tmc_serial_begin());
#endif

#if HAS_TMC_SPI
#if DISABLED(TMC_USE_SW_SPI)
	SETUP_RUN(SPI.begin());
#endif
	SETUP_RUN(tmc_init_cs_pins());
#endif

#if ENABLED(PSU_CONTROL)
	SETUP_LOG("PSU_CONTROL");
	powerManager.init();
#endif

#if ENABLED(POWER_LOSS_RECOVERY)
	SETUP_RUN(recovery.setup());
#endif

#if HAS_STEPPER_RESET
	SETUP_RUN(disableStepperDrivers());
#endif

	SETUP_RUN(hal.init_board());

#if ENABLED(WIFISUPPORT)
	SETUP_RUN(esp_wifi_init());
#endif

	// Report Reset Reason
	if (mcu & RST_POWER_ON)  SERIAL_ECHOLNPGM(STR_POWERUP);
	if (mcu & RST_EXTERNAL)  SERIAL_ECHOLNPGM(STR_EXTERNAL_RESET);
	if (mcu & RST_BROWN_OUT) SERIAL_ECHOLNPGM(STR_BROWNOUT_RESET);
	if (mcu & RST_WATCHDOG)  SERIAL_ECHOLNPGM(STR_WATCHDOG_RESET);
	if (mcu & RST_SOFTWARE)  SERIAL_ECHOLNPGM(STR_SOFTWARE_RESET);

	// Identify myself as Marlin x.x.x
	SERIAL_ECHOLNPGM("Marlin " SHORT_BUILD_VERSION);
#if defined(STRING_DISTRIBUTION_DATE) && defined(STRING_CONFIG_H_AUTHOR)
	SERIAL_ECHO_MSG(
		" Last Updated: " STRING_DISTRIBUTION_DATE
		" | Author: " STRING_CONFIG_H_AUTHOR
	);
#endif
	SERIAL_ECHO_MSG(" Compiled: " __DATE__);
	SERIAL_ECHO_MSG(STR_FREE_MEMORY, hal.freeMemory(), STR_PLANNER_BUFFER_BYTES, sizeof(block_t) * (BLOCK_BUFFER_SIZE));

	// Some HAL need precise delay adjustment
	calibrate_delay_loop();

	// Init buzzer pin(s)
#if HAS_BEEPER
	SETUP_RUN(buzzer.init());
#endif

	// Set up LEDs early
#if HAS_COLOR_LEDS
	SETUP_RUN(leds.setup());
#endif

#if ENABLED(NEOPIXEL2_SEPARATE)
	SETUP_RUN(leds2.setup());
#endif

#if ENABLED(USE_CONTROLLER_FAN)     // Set up fan controller to initialize also the default configurations.
	SETUP_RUN(controllerFan.setup());
#endif

	TERN_(HAS_FANCHECK, fan_check.init());

	// UI must be initialized before EEPROM
	// (because EEPROM code calls the UI).

	SETUP_RUN(ui.init());

#if PIN_EXISTS(SAFE_POWER)
#if HAS_DRIVER_SAFE_POWER_PROTECT
	SETUP_RUN(stepper_driver_backward_check());
#else
	SETUP_LOG("SAFE_POWER");
	OUT_WRITE(SAFE_POWER_PIN, HIGH);
#endif
#endif

#if BOTH(SDSUPPORT, SDCARD_EEPROM_EMULATION)
	SETUP_RUN(card.mount());          // Mount media with settings before first_load
#endif

	SETUP_RUN(settings.first_load());   // Load data from EEPROM if available (or use defaults)
										// This also updates variables in the planner, elsewhere

#if BOTH(HAS_WIRED_LCD, SHOW_BOOTSCREEN)
	SETUP_RUN(ui.show_bootscreen());
	const millis_t bootscreen_ms = millis();
#endif

#if ENABLED(PROBE_TARE)
	SETUP_RUN(probe.tare_init());
#endif

#if HAS_ETHERNET
	SETUP_RUN(ethernet.init());
#endif

#if HAS_TOUCH_BUTTONS
	SETUP_RUN(touchBt.init());
#endif

	TERN_(HAS_M206_COMMAND, current_position += home_offset); // Init current position based on home_offset

	sync_plan_position();               // Vital to init stepper/planner equivalent for current_position

	SETUP_RUN(thermalManager.init());   // Initialize temperature loop

	SETUP_RUN(print_job_timer.init());  // Initial setup of print job timer

	SETUP_RUN(endstops.init());         // Init endstops and pullups

#if ENABLED(DELTA) && !HAS_SOFTWARE_ENDSTOPS
	SETUP_RUN(refresh_delta_clip_start_height()); // Init safe delta height without soft endstops
#endif

	SETUP_RUN(stepper.init());          // Init stepper. This enables interrupts!

#if HAS_SERVOS
	SETUP_RUN(servo_init());
#endif

#if HAS_Z_SERVO_PROBE
	SETUP_RUN(probe.servo_probe_init());
#endif

#if HAS_PHOTOGRAPH
	OUT_WRITE(PHOTOGRAPH_PIN, LOW);
#endif

#if HAS_CUTTER
	SETUP_RUN(cutter.init());
#endif

#if ENABLED(COOLANT_MIST)
	OUT_WRITE(COOLANT_MIST_PIN, COOLANT_MIST_INVERT);   // Init Mist Coolant OFF
#endif
#if ENABLED(COOLANT_FLOOD)
	OUT_WRITE(COOLANT_FLOOD_PIN, COOLANT_FLOOD_INVERT); // Init Flood Coolant OFF
#endif

#if HAS_BED_PROBE
#if PIN_EXISTS(PROBE_ENABLE)
	OUT_WRITE(PROBE_ENABLE_PIN, LOW); // Disable
#endif
	SETUP_RUN(endstops.enable_z_probe(false));
#endif

#if HAS_STEPPER_RESET
	SETUP_RUN(enableStepperDrivers());
#endif

#if HAS_MOTOR_CURRENT_I2C
	SETUP_RUN(digipot_i2c.init());
#endif

#if HAS_MOTOR_CURRENT_DAC
	SETUP_RUN(stepper_dac.init());
#endif

#if EITHER(Z_PROBE_SLED, SOLENOID_PROBE) && HAS_SOLENOID_1
	OUT_WRITE(SOL1_PIN, LOW); // OFF
#endif

#if HAS_HOME
	SET_INPUT_PULLUP(HOME_PIN);
#endif

#if ENABLED(CUSTOM_USER_BUTTONS)
#define INIT_CUSTOM_USER_BUTTON_PIN(N) do{ SET_INPUT(BUTTON##N##_PIN); WRITE(BUTTON##N##_PIN, !BUTTON##N##_HIT_STATE); }while(0)

#if HAS_CUSTOM_USER_BUTTON(1)
	INIT_CUSTOM_USER_BUTTON_PIN(1);
#endif
#if HAS_CUSTOM_USER_BUTTON(2)
	INIT_CUSTOM_USER_BUTTON_PIN(2);
#endif
#if HAS_CUSTOM_USER_BUTTON(3)
	INIT_CUSTOM_USER_BUTTON_PIN(3);
#endif
#if HAS_CUSTOM_USER_BUTTON(4)
	INIT_CUSTOM_USER_BUTTON_PIN(4);
#endif
#if HAS_CUSTOM_USER_BUTTON(5)
	INIT_CUSTOM_USER_BUTTON_PIN(5);
#endif
#if HAS_CUSTOM_USER_BUTTON(6)
	INIT_CUSTOM_USER_BUTTON_PIN(6);
#endif
#if HAS_CUSTOM_USER_BUTTON(7)
	INIT_CUSTOM_USER_BUTTON_PIN(7);
#endif
#if HAS_CUSTOM_USER_BUTTON(8)
	INIT_CUSTOM_USER_BUTTON_PIN(8);
#endif
#if HAS_CUSTOM_USER_BUTTON(9)
	INIT_CUSTOM_USER_BUTTON_PIN(9);
#endif
#if HAS_CUSTOM_USER_BUTTON(10)
	INIT_CUSTOM_USER_BUTTON_PIN(10);
#endif
#if HAS_CUSTOM_USER_BUTTON(11)
	INIT_CUSTOM_USER_BUTTON_PIN(11);
#endif
#if HAS_CUSTOM_USER_BUTTON(12)
	INIT_CUSTOM_USER_BUTTON_PIN(12);
#endif
#if HAS_CUSTOM_USER_BUTTON(13)
	INIT_CUSTOM_USER_BUTTON_PIN(13);
#endif
#if HAS_CUSTOM_USER_BUTTON(14)
	INIT_CUSTOM_USER_BUTTON_PIN(14);
#endif
#if HAS_CUSTOM_USER_BUTTON(15)
	INIT_CUSTOM_USER_BUTTON_PIN(15);
#endif
#if HAS_CUSTOM_USER_BUTTON(16)
	INIT_CUSTOM_USER_BUTTON_PIN(16);
#endif
#if HAS_CUSTOM_USER_BUTTON(17)
	INIT_CUSTOM_USER_BUTTON_PIN(17);
#endif
#if HAS_CUSTOM_USER_BUTTON(18)
	INIT_CUSTOM_USER_BUTTON_PIN(18);
#endif
#if HAS_CUSTOM_USER_BUTTON(19)
	INIT_CUSTOM_USER_BUTTON_PIN(19);
#endif
#if HAS_CUSTOM_USER_BUTTON(20)
	INIT_CUSTOM_USER_BUTTON_PIN(20);
#endif
#if HAS_CUSTOM_USER_BUTTON(21)
	INIT_CUSTOM_USER_BUTTON_PIN(21);
#endif
#if HAS_CUSTOM_USER_BUTTON(22)
	INIT_CUSTOM_USER_BUTTON_PIN(22);
#endif
#if HAS_CUSTOM_USER_BUTTON(23)
	INIT_CUSTOM_USER_BUTTON_PIN(23);
#endif
#if HAS_CUSTOM_USER_BUTTON(24)
	INIT_CUSTOM_USER_BUTTON_PIN(24);
#endif
#if HAS_CUSTOM_USER_BUTTON(25)
	INIT_CUSTOM_USER_BUTTON_PIN(25);
#endif
#endif

#if PIN_EXISTS(STAT_LED_RED)
	OUT_WRITE(STAT_LED_RED_PIN, LOW); // OFF
#endif
#if PIN_EXISTS(STAT_LED_BLUE)
	OUT_WRITE(STAT_LED_BLUE_PIN, LOW); // OFF
#endif

#if ENABLED(CASE_LIGHT_ENABLE)
	SETUP_RUN(caselight.init());
#endif

#if HAS_PRUSA_MMU1
	SETUP_RUN(mmu_init());
#endif

#if HAS_FANMUX
	SETUP_RUN(fanmux_init());
#endif

#if ENABLED(MIXING_EXTRUDER)
	SETUP_RUN(mixer.init());
#endif

#if ENABLED(BLTOUCH)
	SETUP_RUN(bltouch.init(//set_voltage=true));
#endif

#if ENABLED(MAGLEV4)
	OUT_WRITE(MAGLEV_TRIGGER_PIN, LOW);
#endif

#if ENABLED(I2C_POSITION_ENCODERS)
	SETUP_RUN(I2CPEM.init());
#endif

#if ENABLED(EXPERIMENTAL_I2CBUS) && I2C_SLAVE_ADDRESS > 0
	SETUP_LOG("i2c...");
	i2c.onReceive(i2c_on_receive);
	i2c.onRequest(i2c_on_request);
#endif

#if DO_SWITCH_EXTRUDER
	SETUP_RUN(move_extruder_servo(0));  // Initialize extruder servo
#endif

#if ENABLED(SWITCHING_NOZZLE)
	SETUP_LOG("SWITCHING_NOZZLE");
	// Initialize nozzle servo(s)
#if SWITCHING_NOZZLE_TWO_SERVOS
	lower_nozzle(0);
	raise_nozzle(1);
#else
	move_nozzle_servo(0);
#endif
#endif

#if ENABLED(PARKING_EXTRUDER)
	SETUP_RUN(pe_solenoid_init());
#elif ENABLED(MAGNETIC_PARKING_EXTRUDER)
	SETUP_RUN(mpe_settings_init());
#elif ENABLED(SWITCHING_TOOLHEAD)
	SETUP_RUN(swt_init());
#elif ENABLED(ELECTROMAGNETIC_SWITCHING_TOOLHEAD)
	SETUP_RUN(est_init());
#endif

#if ENABLED(USE_WATCHDOG)
	SETUP_RUN(hal.watchdog_init());   // Reinit watchdog after hal.get_reset_source call
#endif

#if ENABLED(EXTERNAL_CLOSED_LOOP_CONTROLLER)
	SETUP_RUN(closedloop.init());
#endif

#ifdef STARTUP_COMMANDS
	SETUP_LOG("STARTUP_COMMANDS");
	queue.inject(F(STARTUP_COMMANDS));
#endif

#if ENABLED(HOST_PROMPT_SUPPORT)
	SETUP_RUN(hostui.prompt_end());
#endif

#if HAS_DRIVER_SAFE_POWER_PROTECT
	SETUP_RUN(stepper_driver_backward_report());
#endif

#if HAS_PRUSA_MMU2
	SETUP_RUN(mmu2.init());
#endif

#if ENABLED(IIC_BL24CXX_EEPROM)
	BL24CXX::init();
	const uint8_t err = BL24CXX::check();
	SERIAL_ECHO_TERNARY(err, "BL24CXX Check ", "failed", "succeeded", "!\n");
#endif

#if HAS_DWIN_E3V2_BASIC
	SETUP_RUN(DWIN_InitScreen());
#endif

#if HAS_SERVICE_INTERVALS && !HAS_DWIN_E3V2_BASIC
	SETUP_RUN(ui.reset_status(true));  // Show service messages or keep current status
#endif

#if ENABLED(MAX7219_DEBUG)
	SETUP_RUN(max7219.init());
#endif

#if ENABLED(DIRECT_STEPPING)
	SETUP_RUN(page_manager.init());
#endif

#if HAS_TFT_LVGL_UI
#if ENABLED(SDSUPPORT)
	if (!card.isMounted()) SETUP_RUN(card.mount()); // Mount SD to load graphics and fonts
#endif
	SETUP_RUN(tft_lvgl_init());
#endif

#if BOTH(HAS_WIRED_LCD, SHOW_BOOTSCREEN)
	const millis_t elapsed = millis() - bootscreen_ms;
#if ENABLED(MARLIN_DEV_MODE)
	SERIAL_ECHOLNPGM("elapsed=", elapsed);
#endif
	SETUP_RUN(ui.bootscreen_completion(elapsed));
#endif

#if ENABLED(PASSWORD_ON_STARTUP)
	SETUP_RUN(password.lock_machine());      // Will not proceed until correct password provided
#endif

#if BOTH(HAS_MARLINUI_MENU, TOUCH_SCREEN_CALIBRATION) && EITHER(TFT_CLASSIC_UI, TFT_COLOR_UI)
	SETUP_RUN(ui.check_touch_calibration());
#endif

#if ENABLED(EASYTHREED_UI)
	SETUP_RUN(easythreed_ui.init());
#endif

#if HAS_TRINAMIC_CONFIG && DISABLED(PSU_DEFAULT_OFF)
	SETUP_RUN(test_tmc_connection());
#endif

#if ENABLED(BD_SENSOR)
	SETUP_RUN(bdl.init(I2C_BD_SDA_PIN, I2C_BD_SCL_PIN, I2C_BD_DELAY));
#endif

	marlin_state = MF_RUNNING;

	SETUP_LOG("setup() completed.");

	TERN_(MARLIN_TEST_BUILD, runStartupTests());
*/
}//setup()
void loop(void) {

}//loop()

//MarlinCore.cpp
