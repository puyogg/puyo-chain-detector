#include <obs-module.h>

OBS_DECLARE_MODULE()

OBS_MODULE_USE_DEFAULT_LOCALE("puyo-chain-detector", "en-US")

extern struct obs_source_info puyo_chain_overlay_filter;

MODULE_EXPORT const char* obs_module_description(void)
{
	return "Puyo Chain Detector plugin";
}

bool obs_module_load()
{
	obs_register_source(&puyo_chain_overlay_filter);
	return true;
}
