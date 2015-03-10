#ifndef SND16_H
#define SND16_H


static attr_value_t
get_wave_type (void *arg, conf_object_t *obj, attr_value_t *idx);
static set_error_t
set_wave_type (void *arg, conf_object_t *obj, attr_value_t *val, attr_value_t *idx);

static attr_value_t
get_signal_freq (void *arg, conf_object_t *obj, attr_value_t *idx);
static set_error_t
set_signal_freq (void *arg, conf_object_t *obj, attr_value_t *val, attr_value_t *idx);

static attr_value_t
get_waveform_limit (void *arg, conf_object_t *obj, attr_value_t *idx);
static set_error_t
set_waveform_limit (void *arg, conf_object_t *obj, attr_value_t *val, attr_value_t *idx);

#endif  // SND16_H
