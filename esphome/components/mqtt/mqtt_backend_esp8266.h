#pragma once

#ifdef USE_ESP8266

#include "mqtt_backend.h"
#include "esphome/core/log.h"
#include <WiFiClientSecure.h>
// #include <WiFiClient.h>
#include <PicoMQTT.h>
#include <string>

#define SHA1_SIZE 20

namespace esphome {
namespace mqtt {

static const char cert_ISRG_X1[] PROGMEM = R"CERT(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)CERT";

static BearSSL::X509List trustedRoots;

class MQTTBackendESP8266 final : public MQTTBackend {
 public:
  MQTTBackendESP8266() : mqtt_client_(wifi_client_) {
    trustedRoots.append(cert_ISRG_X1);
    // wifi_client_.setInsecure();
    // wifi_client_.setSSLVersion(1,2);
    wifi_client_.setBufferSizes(2048, 2048);
    wifi_client_.setTimeout(10000);
    wifi_client_.setTrustAnchors(&trustedRoots);
    mqtt_client_.connected_callback = [this]() { this->on_connect(); };
    mqtt_client_.disconnected_callback = [this]() { this->on_disconnect(); };
  }
  void set_keep_alive(uint16_t keep_alive) final { keep_alive_ = keep_alive; }
  void set_client_id(const char *client_id) final { client_id_ = client_id; }
  void set_clean_session(bool clean_session) final { clean_session_ = clean_session; }
  void set_credentials(const char *username, const char *password) final {
    if (username) {
      username_ = username;
    }
    if (password) {
      password_ = password;
    }
  }
  void set_will(const char *topic, uint8_t qos, bool retain, const char *payload) final {
    if (topic) {
      will_topic_ = topic;
    }
    will_qos_ = qos;
    if (payload) {
      will_payload_ = payload;
    }
    will_retain_ = retain;
  }
  void set_server(network::IPAddress ip, uint16_t port) final {
    server_host_ = ip.str();
    server_port_ = port;
  }
  void set_server(const char *host, uint16_t port) final {
    server_host_ = host;
    server_port_ = port;
  }
#if ASYNC_TCP_SSL_ENABLED
  void set_secure(bool secure) {
    if (!secure) {
      wifi_client_.setInsecure();
    }
  }
  void add_server_fingerprint(const uint8_t *fingerprint) { wifi_client_.setFingerprint(fingerprint); }
#endif
  void set_on_connect(std::function<on_connect_callback_t> &&callback) final {
    on_connect_callbacks_.push_back(callback);
  }
  void set_on_disconnect(std::function<on_disconnect_callback_t> &&callback) final {
    on_disconnect_callbacks_.push_back(callback);
  }
  void set_on_subscribe(std::function<on_subscribe_callback_t> &&callback) final {
    on_subscribe_callbacks_.push_back(callback);
    // not triggered yet but also not used in esphome yet
  }
  void set_on_unsubscribe(std::function<on_unsubscribe_callback_t> &&callback) final {
    on_unsubscribe_callbacks_.push_back(callback);
    // not triggered yet but also not used in esphome yet
  }
  void set_on_message(std::function<on_message_callback_t> &&callback) final {
    on_message_callbacks_.push_back(callback);
  }
  void set_on_publish(std::function<on_publish_user_callback_t> &&callback) final {
    on_publish_callbacks_.push_back(callback);
    // not triggered yet but also not used in esphome yet
  }
  bool connected() const final { return const_cast<PicoMQTT::Client &>(mqtt_client_).connected(); }
  void connect() final {
    ESP_LOGD(MQTTBackendESP8266::TAG, "MQTT_CONNECT");
    mqtt_client_.connect(server_host_.c_str(), server_port_, client_id_.c_str(), username_.c_str(), password_.c_str());
  }
  void disconnect() final {
    ESP_LOGD(MQTTBackendESP8266::TAG, "MQTT_DISCONNECT");
    mqtt_client_.disconnect();
  }
  bool subscribe(const char *topic, uint8_t qos) final {
    if (mqtt_client_.connected()) {
      ESP_LOGD(MQTTBackendESP8266::TAG, "MQTT_SUBSCRIBE");
      mqtt_client_.subscribe(topic, [this](char *topic, void *payload, size_t payload_size) {
        ESP_LOGD(MQTTBackendESP8266::TAG, "MQTT_SUBSCRIBE_CALLBACK");
        this->on_message(topic, payload, payload_size);
      });
      return true;
    }
    ESP_LOGD(MQTTBackendESP8266::TAG, "MQTT_SUBSCRIBE %s %d FAILED", topic, qos);
    return false;
  }
  bool unsubscribe(const char *topic) final {
    ESP_LOGD(MQTTBackendESP8266::TAG, "MQTT_UNSUBSCRIBE");
    mqtt_client_.unsubscribe(topic);
    // there is no return value for unsubscribe
    return true;
  }
  bool publish(const char *topic, const char *payload, size_t length, uint8_t qos, bool retain) final {
    ESP_LOGD(MQTTBackendESP8266::TAG, "MQTT_PUBLISH");
    return mqtt_client_.publish(topic, static_cast<const void *>(payload), length, qos, retain);
  }
  void on_message(const char *topic, const void *payload, size_t payload_size) {
    ESP_LOGD(MQTTBackendESP8266::TAG, "MQTT_MESSAGE %s", topic);
    for (auto &callback : on_message_callbacks_) {
      callback(topic, static_cast<const char *>(payload), payload_size, 0, payload_size);
    }
  }
  void on_connect() {
    ESP_LOGD(MQTTBackendESP8266::TAG, "MQTT_ON_CONNECT");
    for (auto &callback : on_connect_callbacks_) {
      callback(false);
    }
  }
  void on_disconnect() {
    ESP_LOGD(MQTTBackendESP8266::TAG, "MQTT_ON_DISCONNECT");
    for (auto &callback : on_disconnect_callbacks_) {
      callback(MQTTClientDisconnectReason::TCP_DISCONNECTED);
    }
  }
  void loop() final { mqtt_client_.loop(); }

  using MQTTBackend::publish;

 protected:
  PicoMQTT::Client mqtt_client_;
  BearSSL::WiFiClientSecure wifi_client_;
  // WiFiClient wifi_client_;

  std::vector<std::function<on_message_callback_t>> on_message_callbacks_;
  std::vector<std::function<on_connect_callback_t>> on_connect_callbacks_;
  std::vector<std::function<on_disconnect_callback_t>> on_disconnect_callbacks_;
  std::vector<std::function<on_subscribe_callback_t>> on_subscribe_callbacks_;
  std::vector<std::function<on_unsubscribe_callback_t>> on_unsubscribe_callbacks_;
  std::vector<std::function<on_publish_user_callback_t>> on_publish_callbacks_;

  uint16_t server_port_{8883};
  std::string server_host_;
  uint16_t keep_alive_{60};
  std::string client_id_;
  bool clean_session_{true};
  std::string username_;
  std::string password_;
  std::string will_topic_;
  uint8_t will_qos_{0};
  bool will_retain_{false};
  std::string will_payload_;
  bool secure_{false};
  uint8_t server_fingerprint_[SHA1_SIZE];

  static constexpr const char *const TAG = "mqtt.pico";
};

}  // namespace mqtt
}  // namespace esphome

#endif  // defined(USE_ESP8266)
