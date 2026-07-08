#!/usr/bin/env python3
import argparse
import http.server
import ssl
from pathlib import Path


class DroneOpsHandler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header("Cache-Control", "no-store")
        super().end_headers()


def main():
    parser = argparse.ArgumentParser(description="DroneOps local HTTPS static server")
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=8021)
    parser.add_argument("--directory", default=str(Path(__file__).resolve().parent))
    parser.add_argument("--ssl-keyfile", default="")
    parser.add_argument("--ssl-certfile", default="")
    args = parser.parse_args()

    handler = lambda *h_args, **h_kwargs: DroneOpsHandler(  # noqa: E731
        *h_args,
        directory=args.directory,
        **h_kwargs,
    )
    server = http.server.ThreadingHTTPServer((args.host, args.port), handler)
    if args.ssl_keyfile and args.ssl_certfile:
        context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        context.load_cert_chain(certfile=args.ssl_certfile, keyfile=args.ssl_keyfile)
        server.socket = context.wrap_socket(server.socket, server_side=True)

    scheme = "https" if args.ssl_keyfile and args.ssl_certfile else "http"
    print(f"DroneOps server listening on {scheme}://{args.host}:{args.port}", flush=True)
    server.serve_forever()


if __name__ == "__main__":
    main()

