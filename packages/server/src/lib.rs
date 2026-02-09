use poem::{Server, listener::TcpListener};
use std::sync::{Arc, Mutex};
use tokio::sync::mpsc;

mod detector;
mod routes;

use routes::shutdown::ShutdownSignal;

use crate::detector::create_detector;

pub async fn server(host: &str, port: &str, auto_restart: bool) {
    loop {
        let (tx, mut rx) = mpsc::channel::<ShutdownSignal>(16);

        let (config_change_tx, toggle_detector_tx, image_tx, detector) = create_detector();
        let mut detector = detector;

        let router = routes::init_router(
            &format!("http://localhost:{port}"),
            tx.clone(),
            config_change_tx.clone(),
            toggle_detector_tx.clone(),
            image_tx,
        );
        let server = Server::new(TcpListener::bind(format!("{host}:{port}")));
        let restart_behavior = match auto_restart {
            true => ShutdownSignal::Restart,
            false => ShutdownSignal::Shutdown,
        };
        let shutdown_signal = Arc::new(Mutex::new(restart_behavior));
        let shutdown_ref = shutdown_signal.clone();

        tokio::select! {
            _ = server.run_with_graceful_shutdown(
                    router,
                    async move {
                        let response = rx.recv().await;
                        println!("Received shutdown signal.");

                        match response {
                            Some(signal) => match signal {
                                ShutdownSignal::Shutdown => {
                                    let mut locked_shutdown_ref = shutdown_ref.lock().unwrap();
                                    *locked_shutdown_ref = ShutdownSignal::Shutdown;
                                    return ();
                                }
                                ShutdownSignal::Restart => return (),
                            },
                            None => return (),
                        }
                    },
                    None,
                ) => {},
            _ = detector.start() => {},
        }

        let signal = shutdown_signal.lock().unwrap();
        match *signal {
            ShutdownSignal::Shutdown => {
                println!("Shutting down server.");
                break;
            }
            ShutdownSignal::Restart => {
                println!("Restarting server.");
                continue;
            }
        }
    }

    ()
}
