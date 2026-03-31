import json
import socket
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer


TCP_HOST = "0.0.0.0"
TCP_PORT = 8765
WEB_HOST = "0.0.0.0"
WEB_PORT = 8080


state_lock = threading.Lock()
connections = {}
next_conn_id = 1


def now_str() -> str:
    return time.strftime("%Y-%m-%d %H:%M:%S")


def add_connection(addr) -> int:
    global next_conn_id
    with state_lock:
        conn_id = next_conn_id
        next_conn_id += 1
        connections[conn_id] = {
            "id": conn_id,
            "peer": f"{addr[0]}:{addr[1]}",
            "status": "connected",
            "connected_at": now_str(),
            "disconnected_at": "",
            "messages": [],
        }
        return conn_id


def append_message(conn_id: int, data: bytes) -> None:
    text = data.decode("utf-8", errors="replace")
    with state_lock:
        if conn_id in connections:
            connections[conn_id]["messages"].append(
                {"time": now_str(), "text": text}
            )


def close_connection(conn_id: int) -> None:
    with state_lock:
        if conn_id in connections:
            connections[conn_id]["status"] = "disconnected"
            connections[conn_id]["disconnected_at"] = now_str()


def snapshot_state() -> dict:
    with state_lock:
        conn_list = sorted(
            connections.values(),
            key=lambda c: c["id"],
            reverse=True,
        )
        active = sum(1 for c in conn_list if c["status"] == "connected")
        return {
            "tcp": {
                "listen": f"{TCP_HOST}:{TCP_PORT}",
                "active_connections": active,
                "total_connections": len(conn_list),
            },
            "connections": conn_list,
            "server_time": now_str(),
        }


def handle_tcp_client(client: socket.socket, addr) -> None:
    conn_id = add_connection(addr)
    print(f"[TCP] client #{conn_id} connected from {addr[0]}:{addr[1]}", flush=True)
    try:
        while True:
            data = client.recv(4096)
            if not data:
                break
            append_message(conn_id, data)
            # Echo back to keep compatibility with simple test clients.
            client.sendall(data)
    except Exception as exc:
        print(f"[TCP] client #{conn_id} error: {exc}", flush=True)
    finally:
        close_connection(conn_id)
        client.close()
        print(f"[TCP] client #{conn_id} disconnected", flush=True)


def tcp_server_loop() -> None:
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind((TCP_HOST, TCP_PORT))
    server.listen()
    print(f"[TCP] listening on {TCP_HOST}:{TCP_PORT}", flush=True)
    while True:
        client, addr = server.accept()
        t = threading.Thread(target=handle_tcp_client, args=(client, addr), daemon=True)
        t.start()


HTML_PAGE = """<!doctype html>
<html lang="zh-CN">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>TCP Monitor</title>
  <style>
    body { font-family: Arial, sans-serif; margin: 20px; background: #f5f7fb; color: #222; }
    .card { background: #fff; border-radius: 8px; padding: 16px; margin-bottom: 14px; box-shadow: 0 1px 4px rgba(0,0,0,.08); }
    .row { display: flex; gap: 14px; flex-wrap: wrap; }
    .badge { padding: 2px 8px; border-radius: 12px; font-size: 12px; }
    .ok { background: #e6f7ec; color: #1b7f3a; }
    .off { background: #f2f2f2; color: #666; }
    ul { margin: 8px 0 0; padding-left: 18px; }
    li { margin: 4px 0; white-space: pre-wrap; word-break: break-word; }
    .muted { color: #666; font-size: 13px; }
  </style>
</head>
<body>
  <h2>TCP Monitor</h2>
  <div class="card" id="summary">加载中...</div>
  <div id="connections"></div>

  <script>
    async function refresh() {
      try {
        const res = await fetch('/state?_=' + Date.now());
        const data = await res.json();
        render(data);
      } catch (e) {
        document.getElementById('summary').innerHTML = '<b>状态：</b>无法连接到 Web 服务';
      }
    }

    function esc(s) {
      return String(s).replaceAll('&', '&amp;').replaceAll('<', '&lt;').replaceAll('>', '&gt;');
    }

    function render(data) {
      const summary = document.getElementById('summary');
      summary.innerHTML =
        '<div><b>TCP监听:</b> ' + esc(data.tcp.listen) + '</div>' +
        '<div><b>活跃连接:</b> ' + data.tcp.active_connections + '</div>' +
        '<div><b>总连接数:</b> ' + data.tcp.total_connections + '</div>' +
        '<div class="muted">服务器时间: ' + esc(data.server_time) + '</div>';

      const wrap = document.getElementById('connections');
      if (!data.connections.length) {
        wrap.innerHTML = '<div class="card">暂无连接</div>';
        return;
      }

      wrap.innerHTML = data.connections.map(c => {
        const badge = c.status === 'connected'
          ? '<span class="badge ok">CONNECTED</span>'
          : '<span class="badge off">DISCONNECTED</span>';
        const msgList = c.messages.length
          ? '<ul>' + c.messages.map(m => '<li>[' + esc(m.time) + '] ' + esc(m.text) + '</li>').join('') + '</ul>'
          : '<div class="muted">暂无数据</div>';
        return (
          '<div class="card">' +
          '<div><b>连接 #' + c.id + '</b> ' + badge + '</div>' +
          '<div class="muted">客户端: ' + esc(c.peer) + '</div>' +
          '<div class="muted">连接时间: ' + esc(c.connected_at) + '</div>' +
          (c.disconnected_at ? '<div class="muted">断开时间: ' + esc(c.disconnected_at) + '</div>' : '') +
          '<div><b>消息列表:</b></div>' +
          msgList +
          '</div>'
        );
      }).join('');
    }

    refresh();
    setInterval(refresh, 1000);
  </script>
</body>
</html>
"""


class Handler(BaseHTTPRequestHandler):
    def _send_json(self, obj: dict) -> None:
        body = json.dumps(obj, ensure_ascii=False).encode("utf-8")
        self.send_response(200)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self):
        if self.path.startswith("/state"):
            self._send_json(snapshot_state())
            return

        if self.path == "/" or self.path.startswith("/index"):
            body = HTML_PAGE.encode("utf-8")
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        self.send_response(404)
        self.end_headers()

    def log_message(self, fmt, *args):
        return


def main():
    tcp_thread = threading.Thread(target=tcp_server_loop, daemon=True)
    tcp_thread.start()

    httpd = ThreadingHTTPServer((WEB_HOST, WEB_PORT), Handler)
    print(f"[WEB] open http://127.0.0.1:{WEB_PORT}", flush=True)
    httpd.serve_forever()


if __name__ == "__main__":
    main()
