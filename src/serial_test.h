/*
 * serial_test — background task that accepts test-harness commands over
 * USB serial. Lets a host orchestrator drive the device through every
 * feature in sequence and watch for crashes / freezes / regressions.
 *
 * Wire format (newline-terminated ASCII):
 *   K<hex4>   inject keypress code into input_poll queue. e.g. K001B = ESC
 *   S         dump runtime state (heap, uptime, idle, current feat)
 *   T         dump task stack high-water marks
 *   D         read back LCD as RGB565 hex rows + FNV-1a pixel checksum
 *   R         soft reset (ESP.restart)
 *   ?         banner with version
 *
 * S also reports [UI_STATE]. D emits [FRAME_BEGIN], 135 [FRAME_ROW]
 * lines and [FRAME_END]. Capture is serviced on the UI task with a
 * one-row buffer, and pauses UI polling while streaming. Use at menus,
 * not during time-critical operations; panel readback depends on hardware.
 * Replies are prefixed with "[CMD]" or "[STATE]" so the host can grep
 * cleanly past the normal log noise. Cost: one task, 3 KB stack, polls
 * Serial.available every 20 ms.
 */
#pragma once

void serial_test_init(void);
