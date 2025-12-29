# Tailnet Setup for CO2 Fan Controller

This guide explains how to get your CO2 fan controller accessible from anywhere on your Tailscale network (tailnet).

## Overview

Since Tailscale doesn't run natively on ESP32 (yet), we use WireGuard to connect the ESP32 to a Tailscale "subnet router" - a machine on your tailnet that bridges traffic to the ESP32.

```
                                    TAILNET
   +----------+                  +-----------+                +------------+
   |  Phone   |<---- Tailscale ->| Subnet    |<-- WireGuard ->| ESP32-POE  |
   |  Laptop  |                  | Router    |                | CO2 Sensor |
   +----------+                  +-----------+                +------------+
   100.x.x.x                     100.y.y.y                    10.13.13.10
                                 (also 10.13.13.1)
```

## Prerequisites

- A machine already on your tailnet (Linux server, Raspberry Pi, etc.)
- Tailscale admin access to approve subnet routes
- `wg` (WireGuard tools) installed on the subnet router

## Step 1: Set Up the Subnet Router

On your Linux machine that will act as the bridge:

### 1.1 Install WireGuard

```bash
# Debian/Ubuntu
sudo apt install wireguard wireguard-tools

# Fedora
sudo dnf install wireguard-tools
```

### 1.2 Generate WireGuard Keys

```bash
# Generate server (subnet router) keys
wg genkey | sudo tee /etc/wireguard/server_private.key | wg pubkey | sudo tee /etc/wireguard/server_public.key
sudo chmod 600 /etc/wireguard/server_private.key

# Generate client (ESP32) keys
wg genkey | tee esp32_private.key | wg pubkey > esp32_public.key
```

Save the ESP32 private key - you'll need it for `secrets.yaml`.

### 1.3 Create WireGuard Config

Create `/etc/wireguard/wg-esp32.conf`:

```ini
[Interface]
Address = 10.13.13.1/24
ListenPort = 51820
PrivateKey = <contents of /etc/wireguard/server_private.key>

# Enable IP forwarding for this interface
PostUp = iptables -A FORWARD -i %i -j ACCEPT; iptables -A FORWARD -o %i -j ACCEPT
PostDown = iptables -D FORWARD -i %i -j ACCEPT; iptables -D FORWARD -o %i -j ACCEPT

[Peer]
# CO2 Fan Controller
PublicKey = <contents of esp32_public.key>
AllowedIPs = 10.13.13.10/32
```

### 1.4 Enable IP Forwarding

```bash
# Enable permanently
echo 'net.ipv4.ip_forward = 1' | sudo tee -a /etc/sysctl.conf
sudo sysctl -p

# Or temporarily for testing
sudo sysctl -w net.ipv4.ip_forward=1
```

### 1.5 Start WireGuard

```bash
sudo systemctl enable wg-quick@wg-esp32
sudo systemctl start wg-quick@wg-esp32

# Verify it's running
sudo wg show
```

## Step 2: Configure Tailscale Subnet Routing

### 2.1 Advertise the Route

On the subnet router machine:

```bash
# Advertise the WireGuard subnet to your tailnet
sudo tailscale up --advertise-routes=10.13.13.0/24 --accept-routes
```

### 2.2 Approve the Route in Admin Console

1. Go to https://login.tailscale.com/admin/machines
2. Find your subnet router machine
3. Click the `...` menu -> "Edit route settings"
4. Enable the `10.13.13.0/24` route
5. Click "Save"

### 2.3 Accept Routes on Client Devices

On each device that needs to reach the ESP32:

```bash
# Linux/macOS
tailscale up --accept-routes

# Or in Tailscale GUI, enable "Use Tailscale subnets"
```

## Step 3: Configure the ESP32

### 3.1 Gather the Required Values

You need:
- **ESP32 Private Key**: From `esp32_private.key` (Step 1.2)
- **Server Public Key**: From `/etc/wireguard/server_public.key`
- **Server Endpoint**: Your subnet router's Tailscale IP + port (e.g., `100.64.0.5:51820`)

Get your subnet router's Tailscale IP:
```bash
tailscale ip -4
```

### 3.2 Update secrets.yaml

```yaml
wg_private_key: "aBcDeFgHiJkLmNoPqRsTuVwXyZ1234567890abcdefg="
wg_peer_public_key: "xYzAbCdEfGhIjKlMnOpQrStUvWxYz0987654321xyz="
wg_peer_endpoint: "100.64.0.5:51820"
```

### 3.3 Flash the Firmware

```bash
cd esphome/
esphome run co2-fan-controller.yaml
```

## Step 4: Verify Connectivity

### 4.1 Check WireGuard Handshake

On the subnet router:
```bash
sudo wg show
```

You should see a recent handshake timestamp for the ESP32 peer.

### 4.2 Test from Tailnet

From any device on your tailnet:
```bash
# Ping the ESP32
ping 10.13.13.10

# Access the web UI
curl http://10.13.13.10/api/status
```

### 4.3 Access the Dashboard

Open in your browser: `http://10.13.13.10/`

## Troubleshooting

### No Handshake on WireGuard

1. Check the ESP32 has network connectivity (ethernet link light)
2. Verify the endpoint IP is the Tailscale IP, not local IP
3. Check firewall allows UDP 51820 on the subnet router
4. Verify keys are correct (no extra whitespace)

### Can't Reach ESP32 from Tailnet

1. Verify subnet route is approved in Tailscale admin
2. Check `--accept-routes` is enabled on client devices
3. Verify IP forwarding is enabled: `cat /proc/sys/net/ipv4/ip_forward`
4. Check iptables rules are in place: `sudo iptables -L FORWARD`

### ESP32 Can't Reach Internet/Tailnet

1. Check the `peer_allowed_ips` setting
2. Verify NAT is working on subnet router
3. Add masquerading if needed:
   ```bash
   sudo iptables -t nat -A POSTROUTING -s 10.13.13.0/24 -o tailscale0 -j MASQUERADE
   ```

## Security Notes

1. **Keep the ESP32 private key secret** - never commit to git
2. **Use Tailscale ACLs** to restrict which devices can reach the ESP32
3. **The web interface has basic auth** - set a strong password
4. **Consider disabling the web UI** if only using Home Assistant

## Alternative: Native Tailscale (Experimental)

There's an experimental project for running Tailscale natively on ESP32:
https://github.com/alfs/tailscale-iot

This eliminates the need for a subnet router but is still in early development.
Use at your own risk for production deployments.

## Example Tailscale ACL

Add to your Tailscale ACL policy to restrict access:

```json
{
  "acls": [
    {
      "action": "accept",
      "src": ["tag:home-automation"],
      "dst": ["10.13.13.0/24:*"]
    }
  ],
  "tagOwners": {
    "tag:home-automation": ["your-email@example.com"]
  }
}
```

This limits ESP32 access to devices tagged with `home-automation`.
