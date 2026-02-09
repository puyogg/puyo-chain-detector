use poem::web::Data;
use poem_openapi::{Enum, Object, OpenApi, payload::Json, types::ToJSON};
use serde::{Deserialize, Serialize};
use tokio::sync::mpsc::Sender;

#[derive(Clone, Enum, Serialize, Deserialize)]
#[oai(rename_all = "lowercase")]
#[serde(rename_all = "lowercase")]
pub enum ShutdownSignal {
    Shutdown,
    Restart,
}

#[derive(Clone, Object, Serialize)]
pub struct ShutdownRequest {
    signal: ShutdownSignal,
}

pub struct ShutdownRouter;

#[OpenApi(prefix_path = "/shutdown")]
impl ShutdownRouter {
    #[oai(path = "/", method = "post", operation_id = "shutdown")]
    async fn shutdown(
        &self,
        shutdown_tx: Data<&Sender<ShutdownSignal>>,
        signal: Json<ShutdownRequest>,
    ) -> poem::Result<()> {
        println!("Received signal: {}", signal.0.signal.to_json_string());
        match signal.signal {
            ShutdownSignal::Shutdown => {
                if let Err(e) = shutdown_tx.send(ShutdownSignal::Shutdown).await {
                    println!("Shutdown Error: {e}");
                }
            }
            ShutdownSignal::Restart => {
                if let Err(e) = shutdown_tx.send(ShutdownSignal::Restart).await {
                    println!("Shutdown Error: {e}");
                }
            }
        };

        return Ok(());
    }
}
