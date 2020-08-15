#include <obs-module.h>

struct chain_overlay_filter_data
{
	obs_source_t* context;
	int width;
	int height;
};

static const char* puyo_chain_filter_get_name(void* unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text("PuyoChainDetector");
}

static void* puyo_chain_filter_create(obs_data_t* settings, obs_source_t* context)
{
	auto* filter = static_cast<chain_overlay_filter_data*>(bzalloc(sizeof(chain_overlay_filter_data)));
	filter->context = context;
	obs_source_update(context, settings);
	return filter;
}

static void puyo_chain_filter_destroy(void* data)
{
	auto* filter = static_cast<chain_overlay_filter_data*>(data);
	bfree(filter);
}

static void puyo_chain_filter_update(void* data, obs_data_t* settings)
{
	UNUSED_PARAMETER(data);
	UNUSED_PARAMETER(settings);
}

static obs_properties_t* puyo_chain_filter_properties(void* data)
{
	UNUSED_PARAMETER(data);

	obs_properties_t* props = obs_properties_create();
	return props;
}

static void puyo_chain_filter_defaults(obs_data_t* settings)
{
	UNUSED_PARAMETER(settings);
}

static void puyo_chain_filter_tick(void* data, float seconds)
{
	UNUSED_PARAMETER(seconds);

	auto* filter = static_cast<chain_overlay_filter_data*>(data);
	obs_source_t* target = obs_filter_get_target(filter->context);
	filter->width = obs_source_get_base_width(target);
	filter->height = obs_source_get_base_height(target);
}

static obs_source_frame* puyo_chain_filter_video(void* data, struct obs_source_frame* frame)
{
	UNUSED_PARAMETER(data);
	return frame;
}

static uint32_t puyo_chain_filter_width(void* data)
{
	return static_cast<chain_overlay_filter_data*>(data)->width;
}

static uint32_t puyo_chain_filter_height(void* data)
{
	return static_cast<chain_overlay_filter_data*>(data)->height;
}

extern "C" struct obs_source_info puyo_chain_overlay_filter = {
	.id = "puyo_chain_overlay_filter",
	.type = OBS_SOURCE_TYPE_FILTER,
	.output_flags = OBS_SOURCE_VIDEO,
	.get_name = puyo_chain_filter_get_name,
	.create = puyo_chain_filter_create,
	.destroy = puyo_chain_filter_destroy,
	.get_width = puyo_chain_filter_width,
	.get_height = puyo_chain_filter_height,
	.get_defaults = puyo_chain_filter_defaults,
	.get_properties = puyo_chain_filter_properties,
	.update = puyo_chain_filter_update,
	.video_tick = puyo_chain_filter_tick,
	.filter_video = puyo_chain_filter_video,
};
