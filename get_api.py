import socket

def get_ip_address():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        # The IP doesn't need to be reachable; we just need to trigger the system to choose an interface.
        s.connect(('8.8.8.8', 80))
        ip = s.getsockname()[0]
    except Exception as e:
        ip = "Unable to determine IP"
    finally:
        s.close()
    return ip

if __name__ == '__main__':
    print("Local IP Address:", get_ip_address())
