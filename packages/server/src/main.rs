use server::server;

#[tokio::main]
async fn main() {
    server("0.0.0.0", "3000", true).await;
}
