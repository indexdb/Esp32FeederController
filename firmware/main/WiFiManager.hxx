/*
 * SPDX-FileCopyrightText: 2022 Mike Dunston (atanisoft)
 *
 * SPDX-License-Identifier: CC-BY-NC-SA-4.0
 */

#pragma once

#include <cstring>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_netif.h>
#include <freertos/event_groups.h>
#include <string>

/// Automatic management of WiFi connection
class WiFiManager
{
public:
    /// Constructor.
    ///
    /// @param ssid SSID to connect to.
    /// @param password Password to use for connecting to @param ssid.
    /// @param hostname Hostname to assign to this device.
    WiFiManager(const char * const ssid, const char * const password,
                const char * const hostname);
	WiFiManager(const char * const ssid, const char * const password, const char * const hostname,
                const char * const ipaddr,const char * const netmask,
				const char * const gateway);
    /// Destructor.
    ~WiFiManager();

    /// Starts the WiFi connection process.
    ///
    /// @return true upon successful connection, false otherwise.
    ///
    /// NOTE: This method will *NOT* return until the connection process is
    /// complete, either success or failure.
    bool start();

    /// Processes an event raised by ESP-IDF.
    ///
    /// @param ctx Instance of @ref WiFiManager that registered this handler.
    /// @param event_base Event base raised by ESP-IDF.
    /// @param event_id Event ID raised by ESP-IDF.
    /// @param event_data Event data provided by ESP-IDF, can be null.
    static void process_idf_event(void *ctx, esp_event_base_t event_base,
                                  int32_t event_id, void *event_data);
    /// @returns Currently assigned IP address.
    esp_ip4_addr_t get_local_ip();

private:
    /// Log tag to use for this class.
    static constexpr const char * TAG = "wifi_mgr";

    /// Bit designator for wifi_status_event_group which indicates we have
    /// connected to the SSID.
    static constexpr int WIFI_CONNECTED_BIT = BIT0;

    /// Bit designator for wifi_status_event_group which indicates we have
    /// received an IPv4 address via DHCP.
    static constexpr int WIFI_GOTIP_BIT = BIT1;

    /// Allow up to 36 checks to see if we have connected to the SSID and
    /// received an IPv4 address. This allows up to ~3 minutes for the
    /// entire process to complete, in most cases this should be complete
    /// in under 30 seconds.
    static constexpr uint8_t MAX_CONNECTION_CHECK_ATTEMPTS = 36;

    /// Interval at which to check the status of the WiFi connection.
    static constexpr TickType_t CONNECTION_CHECK_INTERVAL = pdMS_TO_TICKS(5000);

    /// SSID to connect to.
    const std::string ssid_;

    /// Password to use for connecting to @ref ssid_.
    const std::string password_;
    
    const std::string hostname_;
    
    const std::string ipaddr_;

    const std::string netmask_;

    const std::string gateway_;

    /// ESP Station network interface in use.
    esp_netif_t *staIface_;
    
    /// Internal event group used to track the IP assignment events.
    EventGroupHandle_t wifiStatus_;

    /// Bit mask used for checking WiFi connection process events.
    uint32_t wifiConnectBitMask_{WIFI_CONNECTED_BIT};
	bool isValidIPv4(const char *IPAddress);

};