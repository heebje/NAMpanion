#define PLUG_NAME                 "NAMpanion"
#define PLUG_MFR                  "Shameless Plugs"
#define PLUG_VERSION_HEX          0x00010000
#define PLUG_VERSION_STR          "1.0.0"
#define PLUG_UNIQUE_ID            654002602
#define PLUG_MFR_ID               3592876363
#define PLUG_URL_STR              "http://shamelessplugs.net"
#define PLUG_EMAIL_STR            "info@shamelessplugs.net"
#define PLUG_COPYRIGHT_STR        "Copyright 2023 Shameless Plugs"
#define PLUG_CLASS_NAME           NAMpanion

#define BUNDLE_NAME               PLUG_NAME
#define BUNDLE_MFR                PLUG_MFR
#define BUNDLE_DOMAIN             "net"

#define SHARED_RESOURCES_SUBPATH  PLUG_NAME

#define PLUG_CHANNEL_IO           "1-1 1-2 2-2"

#define PLUG_LATENCY              0
#define PLUG_TYPE                 0
#define PLUG_DOES_MIDI_IN         0
#define PLUG_DOES_MIDI_OUT        0
#define PLUG_DOES_MPE             0
#define PLUG_DOES_STATE_CHUNKS    0
#define PLUG_HAS_UI               1
#define PLUG_WIDTH                600
#define PLUG_HEIGHT               280
#define PLUG_FPS                  60
#define PLUG_SHARED_RESOURCES     0
#define PLUG_HOST_RESIZE          0

#define AUV2_ENTRY                NAMpanion_Entry
#define AUV2_ENTRY_STR            "NAMpanion_Entry"
#define AUV2_FACTORY              NAMpanion_Factory
#define AUV2_VIEW_CLASS           NAMpanion_View
#define AUV2_VIEW_CLASS_STR       "NAMpanion_View"

#define AAX_TYPE_IDS              'IEF1', 'IEF2'
#define AAX_TYPE_IDS_AUDIOSUITE   'IEA1', 'IEA2'
#define AAX_PLUG_MFR_STR          PLUG_MFR
#define AAX_PLUG_NAME_STR         "NAMpanion\nIPEF"
#define AAX_PLUG_CATEGORY_STR     "Effect"
#define AAX_DOES_AUDIOSUITE       1

#define VST3_SUBCATEGORY          Steinberg::Vst::PlugType::kFxDistortion

#define APP_NUM_CHANNELS          2
#define APP_N_VECTOR_WAIT         0
#define APP_MULT                  1
#define APP_COPY_AUV3             0
#define APP_SIGNAL_VECTOR_SIZE    64

#define ROBOTO_FN                 "Roboto-Regular.ttf"
