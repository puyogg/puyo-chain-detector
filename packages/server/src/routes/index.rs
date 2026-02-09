use poem::{
    FromRequest, IntoResponse, Request, Response,
    web::{Data, StaticFileRequest, StaticFileResponse},
};
use poem_openapi::{
    Object, OpenApi,
    param::Path,
    payload::{Json, PlainText},
};
use serde::{Deserialize, Serialize};
use tokio::sync::mpsc::Sender;
use poem_openapi::ApiResponse;

use include_dir::{Dir, include_dir};

static UI_DIST_DIR: Dir = include_dir!("$CARGO_MANIFEST_DIR/../ui/dist");

pub struct IndexRouter;

#[OpenApi]
impl IndexRouter {
    #[oai(path = "/", method = "get")]
    async fn index(&self, req: &Request) -> poem::Result<Response> {
        let file = UI_DIST_DIR.get_file("index.html");

        match file {
            Some(f) => match f.contents_utf8() {
                Some(f) => {
                    let response = StaticFileRequest::from_request_without_body(&req)
                        .await?
                        .create_response_from_data(f)?
                        .into_response();

                    Ok(response)
                }
                None => todo!(),
            },
            None => todo!(),
        }
    }

    #[oai(path = "/:name", method = "get")]
    async fn name(
        &self,
        req: &Request,
        Path(name): Path<String>,
    ) -> poem::Result<StaticFileResponse> {
        let file = UI_DIST_DIR.get_file(name);

        match file {
            Some(f) => match f.contents_utf8() {
                Some(f) => {
                    let response = StaticFileRequest::from_request_without_body(&req)
                        .await?
                        .create_response_from_data(f)?;

                    Ok(response)
                }
                None => Err(poem::error::NotFoundError.into()),
            },
            None => Err(poem::error::NotFoundError.into()),
        }
    }
}
