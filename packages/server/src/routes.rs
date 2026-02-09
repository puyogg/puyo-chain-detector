pub mod detector;
pub mod shutdown;

use poem::EndpointExt;
use poem::Route;
use poem::endpoint::EmbeddedFilesEndpoint;
use poem::get;
use poem_openapi::OpenApiService;
use rust_embed::RustEmbed;
use tokio::sync::mpsc::Sender;

use crate::routes::detector::get_image;

type ServerRouter = poem::middleware::AddDataEndpoint<
    poem::middleware::AddDataEndpoint<
        poem::middleware::AddDataEndpoint<
            poem::middleware::AddDataEndpoint<Route, Sender<shutdown::ShutdownSignal>>,
            Sender<detector::DetectorConfig>,
        >,
        Sender<detector::DetectorToggle>,
    >,
    tokio::sync::broadcast::Sender<Vec<u8>>,
>;

#[derive(RustEmbed)]
#[folder = "../ui/dist"]
pub struct Files;

pub fn init_router(
    base_url: &str,
    shutdown_tx: Sender<shutdown::ShutdownSignal>,
    detector_config_tx: Sender<detector::DetectorConfig>,
    toggle_detector_tx: Sender<detector::DetectorToggle>,
    image_tx: tokio::sync::broadcast::Sender<Vec<u8>>,
) -> ServerRouter {
    let open_api = OpenApiService::new(
        (shutdown::ShutdownRouter, detector::DetectorRouter),
        "Puyo Chain Detector: Server API",
        "0.1.0",
    )
    .server(&format!("{base_url}/api"));
    let ui = open_api.swagger_ui();
    let spec = open_api.spec();

    Route::new()
        .nest("/", EmbeddedFilesEndpoint::<Files>::new())
        .nest("/api", open_api)
        .nest("/docs", ui)
        .at("/spec", poem::endpoint::make_sync(move |_| spec.clone()))
        .at("/api/detector/image.jpg", get(get_image))
        .data(shutdown_tx)
        .data(detector_config_tx)
        .data(toggle_detector_tx)
        .data(image_tx)
}
