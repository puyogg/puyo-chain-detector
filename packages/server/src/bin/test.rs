use std::io::Cursor;

use nokhwa::{
    Camera,
    pixel_format::RgbFormat,
    utils::{ApiBackend, CameraIndex, RequestedFormat, Resolution},
};

fn main() {
    let camera_info = nokhwa::query(ApiBackend::Auto).unwrap();
    for camera in camera_info {
        let index = camera.index().as_string();
        let name = camera.human_name();
        let description = camera.description();
        let misc = camera.misc();

        println!("{index}: {name} - {description}");
    }

    let index = CameraIndex::Index(2);
    let requested = RequestedFormat::new::<RgbFormat>(
        nokhwa::utils::RequestedFormatType::AbsoluteHighestFrameRate,
    );

    let mut camera = Camera::new(index, requested).unwrap();

    println!("Getting frame?");
    let res = camera.open_stream();
    println!("{}", camera.is_stream_open());
    let frame = camera.frame().unwrap();
    println!("Trying to decode");
    let decoded = frame.decode_image::<RgbFormat>().unwrap();

    println!("Writing PNG");
    let mut png: Vec<u8> = Vec::new();
    decoded
        .write_to(&mut Cursor::new(&mut png), image::ImageFormat::Png)
        .unwrap();
    println!("len: {}", decoded.len());

    std::fs::write("./image.png", png).unwrap();
}
