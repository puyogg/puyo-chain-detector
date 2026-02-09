use crate::routes::detector::{DetectorConfig, DetectorToggle};
use nokhwa::{
    Camera,
    pixel_format::RgbFormat,
    utils::{CameraFormat, CameraIndex, RequestedFormat, Resolution},
};
use std::io::Cursor;
use tokio::sync::mpsc::{Receiver, Sender};

#[derive(Debug)]
pub struct Detector {
    config_change_recv: Receiver<DetectorConfig>,
    config: DetectorConfig,
    toggle: Receiver<DetectorToggle>,
    image_sender: tokio::sync::broadcast::Sender<Vec<u8>>,
}

fn fps_to_microseconds(fps: u64) -> u64 {
    let fraction_of_second = 1.0 / (fps as f64);
    let microseconds = fraction_of_second * 1000.0 * 1000.0;
    return microseconds as u64;
}

impl Detector {
    pub fn new(
        config_change_recv: Receiver<DetectorConfig>,
        toggle_recv: Receiver<DetectorToggle>,
        image_sender: tokio::sync::broadcast::Sender<Vec<u8>>,
    ) -> Self {
        let config = DetectorConfig {
            test: String::from("Test"),
            fps: 60,
        };
        Detector {
            config_change_recv,
            toggle: toggle_recv,
            image_sender,
            config,
        }
    }

    /// Run one iteration of the chain detector.
    async fn detect(&mut self, iter: &mut i32, camera: &mut Camera) {
        let fps = self.config.fps;
        *iter += 1;

        let frame = camera.frame().unwrap();
        let decoded = frame.decode_image::<RgbFormat>().unwrap();

        // JPEG for debugging
        if self.image_sender.receiver_count() > 0 {
            let mut jpg: Vec<u8> = Vec::new();
            decoded
                .write_to(&mut Cursor::new(&mut jpg), image::ImageFormat::Jpeg)
                .unwrap();
            let _ = self.image_sender.send(jpg);
        }
    }

    pub async fn start(&mut self) {
        'main: loop {
            println!("Waiting for the detector start signal.");
            let toggle_signal = self.toggle.recv().await;

            match toggle_signal {
                None => {
                    println!("Detector toggle channel closed.");
                    break;
                }
                Some(signal) => match signal {
                    DetectorToggle::Start => (),
                    DetectorToggle::Stop => continue 'main,
                },
            };

            'config: loop {
                let duration =
                    std::time::Duration::from_micros(fps_to_microseconds(self.config.fps));
                let mut interval = tokio::time::interval(duration);

                let index = CameraIndex::Index(0);
                let requested = RequestedFormat::new::<RgbFormat>(
                    nokhwa::utils::RequestedFormatType::AbsoluteHighestFrameRate,
                );
                let mut camera = Camera::new(index, requested).unwrap();
                let _ = camera.open_stream();

                let mut iter = 0;
                'detector: loop {
                    tokio::select! {
                        _ = interval.tick() => {
                            self.detect(&mut iter, &mut camera).await;
                        },
                        config = self.config_change_recv.recv() => {
                            match config {
                                Some(config) => {
                                    println!("Setting new config");
                                    self.config = config;
                                    continue 'config;
                                },
                                None => continue 'detector,
                            }
                        },
                        toggle_signal = self.toggle.recv() => {
                            match toggle_signal {
                                None => {
                                    println!("Detector toggle channel closed.");
                                    break 'detector;
                                },
                                Some(signal) => match signal {
                                    DetectorToggle::Start => continue 'detector,
                                    DetectorToggle::Stop => {
                                        println!("Stopping detector.");
                                        continue 'main;
                                    },
                                }
                            };
                        },
                    }
                }
            }
        }
    }
}

pub fn create_detector() -> (
    Sender<DetectorConfig>,
    Sender<DetectorToggle>,
    tokio::sync::broadcast::Sender<Vec<u8>>,
    Detector,
) {
    let (tx, rx) = tokio::sync::mpsc::channel::<DetectorConfig>(1);

    let (toggle_tx, toggle_rx) = tokio::sync::mpsc::channel::<DetectorToggle>(1);

    let (image_tx, _) = tokio::sync::broadcast::channel::<Vec<u8>>(16);

    let detector = Detector::new(rx, toggle_rx, image_tx.clone());

    (tx, toggle_tx, image_tx, detector)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn fps_60() {
        let result = fps_to_microseconds(60);
        assert_eq!(result, 16666);
    }
}
