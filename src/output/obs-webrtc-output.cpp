/**
 * @file obs-webrtc-output.cpp
 * @brief OBS Output plugin integration for WebRTC
 *
 * This file implements the OBS output API callbacks and integrates
 * the WebRTCOutput class with OBS Studio.
 */

#include "output/webrtc-output.hpp"
#include <obs-module.h>
#include <memory>
#include <string>

using namespace obswebrtc::output;

/**
 * @brief Private data structure for OBS output
 */
struct webrtc_output_data {
    obs_output_t* output;
    std::unique_ptr<WebRTCOutput> webrtc_output;
    bool active;
};

/**
 * @brief Get output name
 */
static const char* webrtc_output_getname(void* unused) {
    UNUSED_PARAMETER(unused);
    return "WebRTC Output";
}

/**
 * @brief Create output instance
 */
static void* webrtc_output_create(obs_data_t* settings, obs_output_t* output) {
    auto* data = new webrtc_output_data();
    data->output = output;
    data->active = false;

    blog(LOG_INFO, "[WebRTC Output] Created output instance");

    return data;
}

/**
 * @brief Destroy output instance
 */
static void webrtc_output_destroy(void* data_ptr) {
    auto* data = static_cast<webrtc_output_data*>(data_ptr);

    if (data->active) {
        obs_output_end_data_capture(data->output);
        if (data->webrtc_output) {
            data->webrtc_output->stop();
        }
    }

    delete data;

    blog(LOG_INFO, "[WebRTC Output] Destroyed output instance");
}

/**
 * @brief Start output
 */
static bool webrtc_output_start(void* data_ptr) {
    auto* data = static_cast<webrtc_output_data*>(data_ptr);

    blog(LOG_INFO, "[WebRTC Output] Starting output");

    // Get settings
    obs_data_t* settings = obs_output_get_settings(data->output);
    const char* server_url = obs_data_get_string(settings, "server_url");
    const char* video_codec = obs_data_get_string(settings, "video_codec");
    const char* audio_codec = obs_data_get_string(settings, "audio_codec");
    int64_t video_bitrate = obs_data_get_int(settings, "video_bitrate");
    int64_t audio_bitrate = obs_data_get_int(settings, "audio_bitrate");
    obs_data_release(settings);

    // Validate settings
    if (!server_url || strlen(server_url) == 0) {
        blog(LOG_ERROR, "[WebRTC Output] Server URL is not set");
        obs_output_signal_stop(data->output, OBS_OUTPUT_BAD_PATH);
        return false;
    }

    // Create WebRTC output configuration
    WebRTCOutputConfig config;
    config.serverUrl = server_url;

    // Set video codec
    if (strcmp(video_codec, "h264") == 0) {
        config.videoCodec = VideoCodec::H264;
    } else if (strcmp(video_codec, "vp8") == 0) {
        config.videoCodec = VideoCodec::VP8;
    } else if (strcmp(video_codec, "vp9") == 0) {
        config.videoCodec = VideoCodec::VP9;
    } else if (strcmp(video_codec, "av1") == 0) {
        config.videoCodec = VideoCodec::AV1;
    } else {
        config.videoCodec = VideoCodec::H264; // Default
    }

    // Set audio codec
    if (strcmp(audio_codec, "opus") == 0) {
        config.audioCodec = AudioCodec::Opus;
    } else if (strcmp(audio_codec, "aac") == 0) {
        config.audioCodec = AudioCodec::AAC;
    } else {
        config.audioCodec = AudioCodec::Opus; // Default
    }

    // Set bitrates
    config.videoBitrate = video_bitrate > 0 ? static_cast<int>(video_bitrate) : 2500;
    config.audioBitrate = audio_bitrate > 0 ? static_cast<int>(audio_bitrate) : 128;

    // Set callbacks
    config.errorCallback = [data](const std::string& error) {
        blog(LOG_ERROR, "[WebRTC Output] Error: %s", error.c_str());
        obs_output_signal_stop(data->output, OBS_OUTPUT_ERROR);
    };

    config.stateCallback = [data](bool active) {
        blog(LOG_INFO, "[WebRTC Output] State changed: %s", active ? "active" : "inactive");
        if (!active && data->active) {
            obs_output_signal_stop(data->output, OBS_OUTPUT_DISCONNECTED);
        }
    };

    try {
        // Create WebRTC output
        data->webrtc_output = std::make_unique<WebRTCOutput>(config);

        // Start output
        if (!data->webrtc_output->start()) {
            blog(LOG_ERROR, "[WebRTC Output] Failed to start WebRTC output");
            obs_output_signal_stop(data->output, OBS_OUTPUT_CONNECT_FAILED);
            return false;
        }

        // Start data capture
        if (!obs_output_can_begin_data_capture(data->output, 0)) {
            blog(LOG_ERROR, "[WebRTC Output] Cannot begin data capture");
            data->webrtc_output->stop();
            obs_output_signal_stop(data->output, OBS_OUTPUT_ERROR);
            return false;
        }

        obs_output_begin_data_capture(data->output, 0);
        data->active = true;

        blog(LOG_INFO, "[WebRTC Output] Output started successfully");
        return true;

    } catch (const std::exception& e) {
        blog(LOG_ERROR, "[WebRTC Output] Exception: %s", e.what());
        obs_output_signal_stop(data->output, OBS_OUTPUT_ERROR);
        return false;
    }
}

/**
 * @brief Stop output
 */
static void webrtc_output_stop(void* data_ptr, uint64_t ts) {
    UNUSED_PARAMETER(ts);
    auto* data = static_cast<webrtc_output_data*>(data_ptr);

    blog(LOG_INFO, "[WebRTC Output] Stopping output");

    if (data->active) {
        obs_output_end_data_capture(data->output);
        data->active = false;
    }

    if (data->webrtc_output) {
        data->webrtc_output->stop();
        data->webrtc_output.reset();
    }

    blog(LOG_INFO, "[WebRTC Output] Output stopped");
}

/**
 * @brief Receive encoded packet (both video and audio)
 */
static void webrtc_output_encoded_packet(void* data_ptr, struct encoder_packet* packet) {
    auto* data = static_cast<webrtc_output_data*>(data_ptr);

    if (!data->active || !data->webrtc_output) {
        return;
    }

    try {
        // Convert OBS packet to WebRTC packet
        EncodedPacket webrtc_packet;

        // Determine packet type based on OBS packet type
        if (packet->type == OBS_ENCODER_VIDEO) {
            webrtc_packet.type = PacketType::Video;
            webrtc_packet.keyframe = packet->keyframe;
        } else if (packet->type == OBS_ENCODER_AUDIO) {
            webrtc_packet.type = PacketType::Audio;
            webrtc_packet.keyframe = false;
        } else {
            blog(LOG_WARNING, "[WebRTC Output] Unknown packet type: %d", packet->type);
            return;
        }

        webrtc_packet.data.assign(packet->data, packet->data + packet->size);
        webrtc_packet.timestamp = packet->pts;

        // Send packet
        data->webrtc_output->sendPacket(webrtc_packet);
    } catch (const std::exception& e) {
        blog(LOG_ERROR, "[WebRTC Output] Failed to send packet: %s", e.what());
    }
}

/**
 * @brief Get default settings
 */
static void webrtc_output_defaults(obs_data_t* settings) {
    obs_data_set_default_string(settings, "server_url", "");
    obs_data_set_default_string(settings, "video_codec", "h264");
    obs_data_set_default_string(settings, "audio_codec", "opus");
    obs_data_set_default_int(settings, "video_bitrate", 2500);
    obs_data_set_default_int(settings, "audio_bitrate", 128);
}

/**
 * @brief Get properties
 */
static obs_properties_t* webrtc_output_properties(void* unused) {
    UNUSED_PARAMETER(unused);

    obs_properties_t* props = obs_properties_create();

    // Server URL
    obs_properties_add_text(props, "server_url", "Server URL", OBS_TEXT_DEFAULT);

    // Video codec
    obs_property_t* video_codec = obs_properties_add_list(props, "video_codec", "Video Codec",
                                                          OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
    obs_property_list_add_string(video_codec, "H.264", "h264");
    obs_property_list_add_string(video_codec, "VP8", "vp8");
    obs_property_list_add_string(video_codec, "VP9", "vp9");
    obs_property_list_add_string(video_codec, "AV1", "av1");

    // Audio codec
    obs_property_t* audio_codec = obs_properties_add_list(props, "audio_codec", "Audio Codec",
                                                          OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
    obs_property_list_add_string(audio_codec, "Opus", "opus");
    obs_property_list_add_string(audio_codec, "AAC", "aac");

    // Video bitrate
    obs_properties_add_int(props, "video_bitrate", "Video Bitrate (kbps)", 500, 10000, 100);

    // Audio bitrate
    obs_properties_add_int(props, "audio_bitrate", "Audio Bitrate (kbps)", 64, 320, 16);

    return props;
}

/**
 * @brief Register WebRTC output with OBS
 */
void register_webrtc_output() {
    struct obs_output_info webrtc_output_info = {};

    webrtc_output_info.id = "webrtc_output";
    webrtc_output_info.flags = OBS_OUTPUT_AV | OBS_OUTPUT_ENCODED;
    webrtc_output_info.get_name = webrtc_output_getname;
    webrtc_output_info.create = webrtc_output_create;
    webrtc_output_info.destroy = webrtc_output_destroy;
    webrtc_output_info.start = webrtc_output_start;
    webrtc_output_info.stop = webrtc_output_stop;
    webrtc_output_info.encoded_packet = webrtc_output_encoded_packet;
    webrtc_output_info.encoded_video_codecs = "h264";
    webrtc_output_info.encoded_audio_codecs = "opus";
    webrtc_output_info.get_defaults = webrtc_output_defaults;
    webrtc_output_info.get_properties = webrtc_output_properties;

    obs_register_output(&webrtc_output_info);
    blog(LOG_INFO, "[WebRTC Output] Registered WebRTC output plugin");
}
