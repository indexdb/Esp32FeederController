#include <led_strip.h>
#include <led_strip_types.h>
#include <asio.hpp>
#include <esp_log.h>
#include <cstdint>
#include <nvs.h>
#include <nvs_flash.h>
#include <string>
using std::string;
using std::transform;

class RGBLed
{
public:	
	~RGBLed (){};
	RGBLed (GCodeServer &server)
	{
		gpio_set_direction(RGB_LED1_PIN_NUM, GPIO_MODE_OUTPUT);
		gpio_set_direction(RGB_LED2_PIN_NUM, GPIO_MODE_OUTPUT);
		
		Config_RGBLed (RGB_LED1_PIN_NUM,led_strips[0]);
		Config_RGBLed (RGB_LED2_PIN_NUM,led_strips[1]);
		
		server.register_command(RGB_LED_ON_CMD,
                            std::bind(&RGBLed::led_on, this,
                                      std::placeholders::_1));
		server.register_command(RGB_LED_OFF_CMD,
                            std::bind(&RGBLed::led_off, this,
                                      std::placeholders::_1));

	}
	static constexpr const char *const TAG = "RGB_LED";
		
private:
	led_strip_handle_t led_strips[2];	
	template <typename T>
	bool extract_arg(string arg_letter,
									GCodeServer::command_args args,
									T &value)
	{
		auto ent = std::find_if(args.begin(), args.end(),
								[arg_letter](auto &arg)
								{
									string arg_upper = arg;
									transform(arg_upper.begin(), arg_upper.end(),
											  arg_upper.begin(), ::toupper);
									return arg.rfind(arg_letter, 0) != string::npos;
								});
		if (ent != args.end())
		{
			value = std::stoi(ent->substr(arg_letter.length()));
			return true;
		}
		return false;
	}		
	void Config_RGBLed(gpio_num_t led_io,led_strip_handle_t & led_strip)
	{
		/* LED strip initialization with the GPIO and pixels number*/
		led_strip_config_t strip_config = {
			.strip_gpio_num = (uint32_t) led_io, 		// The GPIO that connected to the LED strip's data line
			.max_leds = RGB_LED_COUNT, 					// The number of LEDs in the strip,
			.led_pixel_format = LED_PIXEL_FORMAT_GRB, 	// Pixel format of your LED strip
			.led_model = LED_MODEL_WS2812, 				// LED strip model
			.flags = {.invert_out = false}, 			// whether to invert the output signal (useful when your hardware has a level inverter)
		};

		led_strip_rmt_config_t rmt_config = {
			.clk_src = RMT_CLK_SRC_DEFAULT, 	// different clock source can lead to different power consumption
			.resolution_hz = 10 * 1000 * 1000,	// 10MHz
			.mem_block_symbols = 0, 			// increase the block size can make the LED less flickering
			.flags = {.with_dma = false}, 		// whether to enable the DMA feature
		};	
		
		ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));		
	}
	
	GCodeServer::command_return_type led_off(GCodeServer::command_args args)
	{
		ESP_LOGI(TAG, "LED Off request received");
		int16_t rgb_led = 0;		
		
		if (!extract_arg("L", args, rgb_led) || rgb_led > 2 || rgb_led < 1)
		{
			return std::make_pair(false, "Missing or Bad RGB LED ID");
		}		
		
		extract_arg("L", args, rgb_led);
		
		led_strip_clear(led_strips[rgb_led-1]);		
		return std::make_pair(true, "");
	}
	
	GCodeServer::command_return_type led_on(GCodeServer::command_args args)
	{
		ESP_LOGI(TAG, "LED On request received");
		
		int16_t rgb_led = 0;
		int16_t r=0,g=0,b=0;
	
		if (!extract_arg("L", args, rgb_led) || rgb_led > 2 || rgb_led < 1)
		{
			return std::make_pair(false, "Missing or Bad RGB LED ID");
		}
		if (!extract_arg("R", args, r) || r <0 || r > 255)
		{
			return std::make_pair(false, "R must be 0-255.");
		}
 		if (!extract_arg("G", args, g) || g <0 || g > 255)
		{
			return std::make_pair(false, "G must be 0-255.");
		}
		if (!extract_arg("B", args, b) || b <0 || b > 255)
		{
			return std::make_pair(false, "B must be 0-255.");
		}
	
		extract_arg("L", args, rgb_led);
		extract_arg("R", args, r);
		extract_arg("G", args, g);
		extract_arg("B", args, b);
		
		ESP_LOGI(TAG, "RGB_LED:%d R:%d G:%d B:%d",rgb_led,r,g,b);
		
		ESP_LOGI(TAG, "RGB_LED_COUNT=%d",RGB_LED_COUNT);
		
		for (uint8_t LedNo = 0; LedNo < RGB_LED_COUNT; LedNo++)
			led_strip_set_pixel(led_strips[rgb_led-1], LedNo, r, g, b);
		
		led_strip_refresh(led_strips[rgb_led-1]);
		return std::make_pair(true, "");
	}
	
protected:
    
    /// Command ID for RGB Led
    static constexpr const char *const RGB_LED_ON_CMD  = "M810";	
	static constexpr const char *const RGB_LED_OFF_CMD = "M811";	
};