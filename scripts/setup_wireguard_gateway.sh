#!/bin/bash
# WireGuard Gateway Setup for ESP32 CO2 Controller
# Run with: sudo bash setup_wireguard_gateway.sh

set -e

echo "=== Setting up WireGuard Gateway for ESP32 ==="

# 1. Create WireGuard config
echo "[1/4] Creating WireGuard config..."
cat > /etc/wireguard/wg0.conf << 'EOF'
[Interface]
Address = 10.13.13.1/24
ListenPort = 51820
PrivateKey = 8NuRLXvPCXhu8/EjalaEAx72/SH9Ra1mMin/KDeOpHA=
PostUp = iptables -A FORWARD -i wg0 -j ACCEPT; iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
PostDown = iptables -D FORWARD -i wg0 -j ACCEPT; iptables -t nat -D POSTROUTING -o eth0 -j MASQUERADE

[Peer]
# ESP32 CO2 Controller
PublicKey = Ez64/qvrDnHaUo54fl1eonh3FziOQOidYUXlnec3LUU=
AllowedIPs = 10.13.13.10/32
EOF
chmod 600 /etc/wireguard/wg0.conf
echo "    Created /etc/wireguard/wg0.conf"

# 2. Enable IP forwarding
echo "[2/4] Enabling IP forwarding..."
if ! grep -q "net.ipv4.ip_forward=1" /etc/sysctl.conf; then
    echo 'net.ipv4.ip_forward=1' >> /etc/sysctl.conf
fi
sysctl -p
echo "    IP forwarding enabled"

# 3. Start WireGuard
echo "[3/4] Starting WireGuard..."
systemctl enable wg-quick@wg0
systemctl start wg-quick@wg0
echo "    WireGuard started"

# 4. Advertise subnet via Tailscale
echo "[4/4] Advertising subnet via Tailscale..."
tailscale up --advertise-routes=10.13.13.0/24 --accept-routes
echo "    Subnet 10.13.13.0/24 advertised"

echo ""
echo "=== Setup Complete ==="
echo ""
wg show
echo ""
echo "ESP32 Configuration (for esphome/secrets.yaml):"
echo "  wg_private_key: \"yIDEWW2sQshA00vdjqjXFOX7Rd2O1KKdupkFZIyoAkY=\""
echo "  wg_peer_public_key: \"1Bmvh9lVXNmFmcxJovlnKwnOxdchdDbtzLpz7zrFVwM=\""
echo "  wg_peer_endpoint: \"100.110.137.103:51820\""
echo ""
echo "Don't forget to approve the subnet route in Tailscale admin console!"
echo "https://login.tailscale.com/admin/machines"
