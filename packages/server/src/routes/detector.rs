use poem::{Body, handler, web::Data};
use poem_openapi::{
    Object, OpenApi,
    payload::{Json, PlainText},
};
use serde::{Deserialize, Serialize};
use tokio::sync::mpsc::Sender;
use tokio_stream::StreamExt;

#[derive(Debug, Clone, Object, Serialize, Deserialize)]
pub struct DetectorConfig {
    pub test: String,
    pub fps: u64,
}

#[derive(Debug)]
pub enum DetectorToggle {
    Start,
    Stop,
}

pub struct DetectorRouter;

#[OpenApi(prefix_path = "/detector")]
impl DetectorRouter {
    #[oai(path = "/config", method = "post", operation_id = "updateConfig")]
    async fn update_config(
        &self,
        detector_config_tx: Data<&Sender<DetectorConfig>>,
        config: Json<DetectorConfig>,
    ) -> poem::Result<PlainText<String>> {
        if let Err(e) = detector_config_tx.send(config.0).await {
            return Err(poem::error::InternalServerError(e));
        }

        Ok(PlainText("OK".to_string()))
    }

    #[oai(path = "/start", method = "post", operation_id = "startDetector")]
    async fn start_detector(
        &self,
        detector_toggle_tx: Data<&Sender<DetectorToggle>>,
    ) -> poem::Result<PlainText<String>> {
        if let Err(e) = detector_toggle_tx.send(DetectorToggle::Start).await {
            return Err(poem::error::InternalServerError(e));
        }

        Ok(PlainText("OK".to_string()))
    }

    #[oai(path = "/stop", method = "post", operation_id = "stopDetector")]
    async fn stop_detector(
        &self,
        detector_toggle_tx: Data<&Sender<DetectorToggle>>,
    ) -> poem::Result<PlainText<String>> {
        if let Err(e) = detector_toggle_tx.send(DetectorToggle::Stop).await {
            return Err(poem::error::InternalServerError(e));
        }

        Ok(PlainText("OK".to_string()))
    }
}

#[handler]
pub async fn get_image(image_tx: Data<&tokio::sync::broadcast::Sender<Vec<u8>>>) -> poem::Response {
    let image_rx = image_tx.subscribe();

    let stream = tokio_stream::wrappers::BroadcastStream::new(image_rx);
    let stream = stream.map(|result| {
        result
            .map(|image_bytes: Vec<u8>| {
                let mut image_bytes_copy = image_bytes.clone();
                let mut multipart_prefix = "--frame\r\nContent-Type: image/jpeg\r\n\r\n"
                    .as_bytes()
                    .to_vec();
                let mut multipart_suffix = "\r\n".as_bytes().to_vec();

                multipart_prefix.append(&mut image_bytes_copy);
                multipart_prefix.append(&mut multipart_suffix);

                return multipart_prefix;
            })
            .map_err(|e| std::io::Error::new(std::io::ErrorKind::Other, e.to_string()))
    });

    let res = poem::Response::builder()
        .content_type("multipart/x-mixed-replace; boundary=frame")
        .body(Body::from_bytes_stream(stream));

    return res;
}
