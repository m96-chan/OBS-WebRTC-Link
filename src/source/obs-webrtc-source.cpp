/**
 * @file obs-webrtc-source.cpp
 * @brief OBS Source plugin integration implementation
 */

#include "obs-webrtc-source.hpp"
#include "webrtc-source.hpp"
#include <obs-module.h>
#include <graphics/graphics.h>
#include <mutex>
#include <queue>

#ifdef ENABLE_QT_UI
#include "ui/settings-dialog.hpp"
#include <QWidget>
#endif

using namespace obswebrtc::source;

/**
 * @brief Source data structure
 */
struct webrtc_source_data {
    obs_source_t *source;
    WebRTCSource *webrtc_source;
    gs_texture_t *texture;

    // Video frame queue
    std::mutex video_mutex;
    std::queue<VideoFrame> video_queue;

    // Audio frame queue
    std::mutex audio_mutex;
    std::queue<AudioFrame> audio_queue;

    // Configuration
    std::string connection_mode;  // "WHEP" or "P2P"
    std::string server_url;
    std::string stream_id;
    std::string auth_token;
    std::string session_id;  // For P2P mode
    VideoCodec video_codec;
    AudioCodec audio_codec;

    // Audio-only mode
    bool audio_only;
    std::string audio_quality;  // "Low", "Medium", "High"

    uint32_t width;
    uint32_t height;
};

/**
 * @brief Get source name
 */
static const char *webrtc_source_get_name(void *unused)
{
    UNUSED_PARAMETER(unused);
    return obs_module_text("WebRTC Link Source");
}

/**
 * @brief Create source
 */
static void *webrtc_source_create(obs_data_t *settings, obs_source_t *source)
{
    auto *data = new webrtc_source_data();
    data->source = source;
    data->texture = nullptr;
    data->width = 1920;
    data->height = 1080;

    // Get settings
    data->connection_mode = obs_data_get_string(settings, "connection_mode");
    data->server_url = obs_data_get_string(settings, "server_url");
    data->stream_id = obs_data_get_string(settings, "stream_id");
    data->auth_token = obs_data_get_string(settings, "auth_token");
    data->session_id = obs_data_get_string(settings, "session_id");
    data->audio_only = obs_data_get_bool(settings, "audio_only");
    data->audio_quality = obs_data_get_string(settings, "audio_quality");
    const char *codec_str = obs_data_get_string(settings, "video_codec");

    if (strcmp(codec_str, "H264") == 0) {
        data->video_codec = VideoCodec::H264;
    } else if (strcmp(codec_str, "VP8") == 0) {
        data->video_codec = VideoCodec::VP8;
    } else if (strcmp(codec_str, "VP9") == 0) {
        data->video_codec = VideoCodec::VP9;
    } else {
        data->video_codec = VideoCodec::H264;
    }

    data->audio_codec = AudioCodec::Opus;

    // Create WebRTC source
    WebRTCSourceConfig config;
    config.serverUrl = data->server_url;
    config.videoCodec = data->video_codec;
    config.audioCodec = data->audio_codec;

    // Set video callback
    config.videoCallback = [data](const VideoFrame& frame) {
        std::lock_guard<std::mutex> lock(data->video_mutex);
        data->video_queue.push(frame);

        // Update dimensions
        data->width = frame.width;
        data->height = frame.height;
    };

    // Set audio callback
    config.audioCallback = [data](const AudioFrame& frame) {
        std::lock_guard<std::mutex> lock(data->audio_mutex);
        data->audio_queue.push(frame);
    };

    // Set error callback
    config.errorCallback = [source](const std::string& error) {
        blog(LOG_ERROR, "[WebRTC Source] Error: %s", error.c_str());
    };

    // Set state callback
    config.stateCallback = [source](ConnectionState state) {
        const char *state_str = "Unknown";
        switch (state) {
            case ConnectionState::Disconnected:
                state_str = "Disconnected";
                break;
            case ConnectionState::Connecting:
                state_str = "Connecting";
                break;
            case ConnectionState::Connected:
                state_str = "Connected";
                break;
            case ConnectionState::Failed:
                state_str = "Failed";
                break;
        }
        blog(LOG_INFO, "[WebRTC Source] State changed: %s", state_str);
    };

    try {
        data->webrtc_source = new WebRTCSource(config);
    } catch (const std::exception& e) {
        blog(LOG_ERROR, "[WebRTC Source] Failed to create source: %s", e.what());
        delete data;
        return nullptr;
    }

    blog(LOG_INFO, "[WebRTC Source] Source created: %s", data->server_url.c_str());

    return data;
}

/**
 * @brief Destroy source
 */
static void webrtc_source_destroy(void *data)
{
    auto *source_data = static_cast<webrtc_source_data*>(data);

    if (source_data->webrtc_source) {
        source_data->webrtc_source->stop();
        delete source_data->webrtc_source;
    }

    obs_enter_graphics();
    if (source_data->texture) {
        gs_texture_destroy(source_data->texture);
    }
    obs_leave_graphics();

    delete source_data;

    blog(LOG_INFO, "[WebRTC Source] Source destroyed");
}

/**
 * @brief Get source width
 */
static uint32_t webrtc_source_get_width(void *data)
{
    auto *source_data = static_cast<webrtc_source_data*>(data);
    return source_data->width;
}

/**
 * @brief Get source height
 */
static uint32_t webrtc_source_get_height(void *data)
{
    auto *source_data = static_cast<webrtc_source_data*>(data);
    return source_data->height;
}

/**
 * @brief Update source settings
 */
static void webrtc_source_update(void *data, obs_data_t *settings)
{
    auto *source_data = static_cast<webrtc_source_data*>(data);

    const char *new_url = obs_data_get_string(settings, "server_url");

    if (source_data->server_url != new_url) {
        source_data->server_url = new_url;

        // Restart source with new settings
        if (source_data->webrtc_source) {
            source_data->webrtc_source->stop();
            delete source_data->webrtc_source;

            WebRTCSourceConfig config;
            config.serverUrl = source_data->server_url;
            config.videoCodec = source_data->video_codec;
            config.audioCodec = source_data->audio_codec;

            try {
                source_data->webrtc_source = new WebRTCSource(config);
            } catch (const std::exception& e) {
                blog(LOG_ERROR, "[WebRTC Source] Failed to recreate source: %s", e.what());
                source_data->webrtc_source = nullptr;
            }
        }
    }
}

/**
 * @brief Get source defaults
 */
static void webrtc_source_get_defaults(obs_data_t *settings)
{
    obs_data_set_default_string(settings, "connection_mode", "WHEP");
    obs_data_set_default_string(settings, "server_url", "");
    obs_data_set_default_string(settings, "stream_id", "");
    obs_data_set_default_string(settings, "auth_token", "");
    obs_data_set_default_string(settings, "session_id", "");
    obs_data_set_default_bool(settings, "audio_only", false);
    obs_data_set_default_string(settings, "audio_quality", "Medium");
    obs_data_set_default_string(settings, "video_codec", "H264");
    obs_data_set_default_int(settings, "video_bitrate", 2500);
    obs_data_set_default_string(settings, "audio_codec", "opus");
    obs_data_set_default_int(settings, "audio_bitrate", 128);
}

#ifdef ENABLE_QT_UI
/**
 * @brief Handle settings dialog button click
 */
static bool webrtc_source_open_settings(obs_properties_t *props, obs_property_t *property, void *data)
{
    UNUSED_PARAMETER(props);
    UNUSED_PARAMETER(property);

    auto *source_data = static_cast<webrtc_source_data*>(data);
    if (!source_data) {
        return false;
    }

    // Get current settings
    obs_data_t *settings = obs_source_get_settings(source_data->source);

    // Create and show settings dialog
    SettingsDialog dialog(nullptr);

    // Load current settings into dialog
    const char *server_url = obs_data_get_string(settings, "server_url");
    const char *video_codec = obs_data_get_string(settings, "video_codec");
    const char *connection_mode = obs_data_get_string(settings, "connection_mode");
    int video_bitrate = obs_data_get_int(settings, "video_bitrate");
    const char *audio_codec = obs_data_get_string(settings, "audio_codec");
    int audio_bitrate = obs_data_get_int(settings, "audio_bitrate");
    const char *token = obs_data_get_string(settings, "token");
    const char *session_id = obs_data_get_string(settings, "session_id");

    dialog.setServerUrl(QString::fromUtf8(server_url));
    dialog.setVideoCodec(QString::fromUtf8(video_codec).toLower());
    dialog.setConnectionMode(QString::fromUtf8(connection_mode));
    dialog.setVideoBitrate(video_bitrate);
    dialog.setAudioCodec(QString::fromUtf8(audio_codec).toLower());
    dialog.setAudioBitrate(audio_bitrate);
    dialog.setToken(QString::fromUtf8(token));
    dialog.setSessionId(QString::fromUtf8(session_id));

    // Show dialog and save settings if accepted
    if (dialog.exec() == QDialog::Accepted) {
        obs_data_set_string(settings, "server_url", dialog.getServerUrl().toUtf8().constData());
        obs_data_set_string(settings, "video_codec", dialog.getVideoCodec().toUtf8().constData());
        obs_data_set_string(settings, "connection_mode", dialog.getConnectionMode().toUtf8().constData());
        obs_data_set_int(settings, "video_bitrate", dialog.getVideoBitrate());
        obs_data_set_string(settings, "audio_codec", dialog.getAudioCodec().toUtf8().constData());
        obs_data_set_int(settings, "audio_bitrate", dialog.getAudioBitrate());
        obs_data_set_string(settings, "token", dialog.getToken().toUtf8().constData());
        obs_data_set_string(settings, "session_id", dialog.getSessionId().toUtf8().constData());

        // Trigger source update
        obs_source_update(source_data->source, settings);
    }

    obs_data_release(settings);
    return true;
}
#endif

/**
 * @brief Get source properties
 */
/**
 * @brief Connection mode changed callback
 */
static bool webrtc_source_connection_mode_changed(obs_properties_t *props, obs_property_t *property, obs_data_t *settings)
{
    const char *mode = obs_data_get_string(settings, "connection_mode");
    bool is_whep = (strcmp(mode, "WHEP") == 0);
    bool is_p2p = (strcmp(mode, "P2P") == 0);

    // Show/hide WHEP fields
    obs_property_set_visible(obs_properties_get(props, "server_url"), is_whep);
    obs_property_set_visible(obs_properties_get(props, "stream_id"), is_whep);
    obs_property_set_visible(obs_properties_get(props, "auth_token"), is_whep);

    // Show/hide P2P fields
    obs_property_set_visible(obs_properties_get(props, "session_id"), is_p2p);

    return true;
}

/**
 * @brief Audio-only mode changed callback
 */
static bool webrtc_source_audio_only_changed(obs_properties_t *props, obs_property_t *property, obs_data_t *settings)
{
    bool audio_only = obs_data_get_bool(settings, "audio_only");

    // Show/hide video codec when audio-only is enabled
    obs_property_set_visible(obs_properties_get(props, "video_codec"), !audio_only);

    // Show/hide audio quality when audio-only is enabled
    obs_property_set_visible(obs_properties_get(props, "audio_quality"), audio_only);

    return true;
}

static obs_properties_t *webrtc_source_get_properties(void *data)
{
    obs_properties_t *props = obs_properties_create();

#ifdef ENABLE_QT_UI
    // Add button to open advanced settings dialog
    obs_properties_add_button(props, "open_settings",
                             obs_module_text("Advanced Settings..."),
                             webrtc_source_open_settings);

    // Add separator
    obs_properties_add_text(props, "separator1", "---- Basic Settings ----", OBS_TEXT_INFO);
#endif

    // Connection Mode dropdown
    obs_property_t *mode = obs_properties_add_list(props, "connection_mode",
                                                    obs_module_text("Connection Mode"),
                                                    OBS_COMBO_TYPE_LIST,
                                                    OBS_COMBO_FORMAT_STRING);
    obs_property_list_add_string(mode, "WHEP (SFU)", "WHEP");
    obs_property_list_add_string(mode, "P2P Client", "P2P");
    obs_property_set_modified_callback(mode, webrtc_source_connection_mode_changed);

    // WHEP Mode settings
    obs_properties_add_text(props, "server_url",
                           obs_module_text("Server URL"),
                           OBS_TEXT_DEFAULT);

    obs_properties_add_text(props, "stream_id",
                           obs_module_text("Stream ID (optional)"),
                           OBS_TEXT_DEFAULT);

    obs_properties_add_text(props, "auth_token",
                           obs_module_text("Bearer Token (optional)"),
                           OBS_TEXT_PASSWORD);

    // P2P Mode settings
    obs_properties_add_text(props, "session_id",
                           obs_module_text("Session ID (from host)"),
                           OBS_TEXT_DEFAULT);

    // Audio-only mode
    obs_property_t *audio_only = obs_properties_add_bool(props, "audio_only",
                                                          obs_module_text("Audio-only Mode"));
    obs_property_set_modified_callback(audio_only, webrtc_source_audio_only_changed);

    // Audio Quality (for audio-only mode)
    obs_property_t *audio_quality = obs_properties_add_list(props, "audio_quality",
                                                             obs_module_text("Audio Quality"),
                                                             OBS_COMBO_TYPE_LIST,
                                                             OBS_COMBO_FORMAT_STRING);
    obs_property_list_add_string(audio_quality, "Low (32 kbps)", "Low");
    obs_property_list_add_string(audio_quality, "Medium (48 kbps)", "Medium");
    obs_property_list_add_string(audio_quality, "High (64 kbps)", "High");

    // Video Codec
    obs_property_t *codec = obs_properties_add_list(props, "video_codec",
                                                     obs_module_text("Video Codec"),
                                                     OBS_COMBO_TYPE_LIST,
                                                     OBS_COMBO_FORMAT_STRING);
    obs_property_list_add_string(codec, "H.264", "H264");
    obs_property_list_add_string(codec, "VP8", "VP8");
    obs_property_list_add_string(codec, "VP9", "VP9");

    return props;
}

/**
 * @brief Show source
 */
static void webrtc_source_show(void *data)
{
    auto *source_data = static_cast<webrtc_source_data*>(data);

    if (source_data->webrtc_source && !source_data->webrtc_source->isActive()) {
        source_data->webrtc_source->start();
        blog(LOG_INFO, "[WebRTC Source] Source started");
    }
}

/**
 * @brief Hide source
 */
static void webrtc_source_hide(void *data)
{
    auto *source_data = static_cast<webrtc_source_data*>(data);

    if (source_data->webrtc_source && source_data->webrtc_source->isActive()) {
        source_data->webrtc_source->stop();
        blog(LOG_INFO, "[WebRTC Source] Source stopped");
    }
}

/**
 * @brief Video tick (called every frame)
 */
static void webrtc_source_video_tick(void *data, float seconds)
{
    UNUSED_PARAMETER(seconds);

    auto *source_data = static_cast<webrtc_source_data*>(data);

    // Process audio frames
    {
        std::lock_guard<std::mutex> lock(source_data->audio_mutex);
        while (!source_data->audio_queue.empty()) {
            const AudioFrame& frame = source_data->audio_queue.front();

            // Convert to OBS audio format
            obs_source_audio audio_data = {};
            audio_data.data[0] = frame.data.data();
            audio_data.frames = frame.data.size() / (sizeof(float) * frame.channels);
            audio_data.speakers = frame.channels == 2 ? SPEAKERS_STEREO : SPEAKERS_MONO;
            audio_data.samples_per_sec = frame.sampleRate;
            audio_data.format = AUDIO_FORMAT_FLOAT;
            audio_data.timestamp = frame.timestamp;

            obs_source_output_audio(source_data->source, &audio_data);

            source_data->audio_queue.pop();
        }
    }
}

/**
 * @brief Video render (called every frame)
 */
static void webrtc_source_video_render(void *data, gs_effect_t *effect)
{
    UNUSED_PARAMETER(effect);

    auto *source_data = static_cast<webrtc_source_data*>(data);

    // Process video frames
    {
        std::lock_guard<std::mutex> lock(source_data->video_mutex);
        if (!source_data->video_queue.empty()) {
            const VideoFrame& frame = source_data->video_queue.front();

            // Create or update texture
            if (!source_data->texture ||
                gs_texture_get_width(source_data->texture) != frame.width ||
                gs_texture_get_height(source_data->texture) != frame.height) {

                if (source_data->texture) {
                    gs_texture_destroy(source_data->texture);
                }

                source_data->texture = gs_texture_create(
                    frame.width, frame.height,
                    GS_RGBA, 1, nullptr, GS_DYNAMIC
                );
            }

            // Update texture with frame data
            if (source_data->texture) {
                uint8_t *tex_data;
                uint32_t linesize;
                if (gs_texture_map(source_data->texture, &tex_data, &linesize)) {
                    // TODO: Proper YUV to RGB conversion
                    // For now, just copy the data
                    memcpy(tex_data, frame.data.data(),
                           std::min(frame.data.size(), (size_t)(linesize * frame.height)));
                    gs_texture_unmap(source_data->texture);
                }
            }

            source_data->video_queue.pop();
        }
    }

    // Render texture
    if (source_data->texture) {
        gs_effect_t *eff = obs_get_base_effect(OBS_EFFECT_DEFAULT);
        gs_eparam_t *image = gs_effect_get_param_by_name(eff, "image");
        gs_effect_set_texture(image, source_data->texture);

        while (gs_effect_loop(eff, "Draw")) {
            gs_draw_sprite(source_data->texture, 0,
                          source_data->width, source_data->height);
        }
    }
}

/**
 * @brief Register WebRTC source with OBS
 */
void register_webrtc_source()
{
    struct obs_source_info info = {};

    info.id = "webrtc_link_source";
    info.type = OBS_SOURCE_TYPE_INPUT;
    info.output_flags = OBS_SOURCE_ASYNC_VIDEO | OBS_SOURCE_AUDIO;

    info.get_name = webrtc_source_get_name;
    info.create = webrtc_source_create;
    info.destroy = webrtc_source_destroy;
    info.get_width = webrtc_source_get_width;
    info.get_height = webrtc_source_get_height;
    info.update = webrtc_source_update;
    info.get_defaults = webrtc_source_get_defaults;
    info.get_properties = webrtc_source_get_properties;
    info.show = webrtc_source_show;
    info.hide = webrtc_source_hide;
    info.video_tick = webrtc_source_video_tick;
    info.video_render = webrtc_source_video_render;

    obs_register_source(&info);

    blog(LOG_INFO, "[WebRTC Source] Source registered");
}
