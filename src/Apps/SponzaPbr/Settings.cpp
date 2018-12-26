#include "Settings.h"
#include <cmath>

Settings::Settings()
{
    msaa_count = 1;
    width = 1280;
    height = 720;
    gamma_correction = true;
    use_reinhard_tone_operator = false;
    use_tone_mapping = true;
    use_white_balance = true;
    use_filmic_hdr = true;
    use_avg_lum = false;
    use_ao = true;
    use_ssao = true;
    use_white_ligth = true;
    use_IBL_diffuse = false;
    use_IBL_specular = false;
    skip_sponza_model = false;
    only_ambient = false;
    light_in_camera = false;
    additional_lights = true;
    show_only_albedo = false;
    show_only_normal = false;
    show_only_roughness = false;
    show_only_metalness = false;
    show_only_ao = false;
    use_f0_with_roughness = false;
    use_flip_normal_y = false;
    use_spec_ao_by_ndotv_roughness = true;
    irradiance_conversion_every_frame = false;
    ambient_power = 0.01;
    light_power = 1.0;// acos(-1.0);
    exposure = 1;
    white = 1;
    s_near = 0.5;
    s_far = 1024.0;
    s_size = 3072;
    use_shadow = false;
    normal_mapping = true;
    shadow_discard = true;
    dynamic_sun_position = false;
}
