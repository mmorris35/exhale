#!/bin/bash
# Home Assistant Docker Setup
# Run with: sudo bash setup_homeassistant.sh

set -e

echo "=== Setting up Home Assistant on Docker ==="

# 1. Install Docker if not present
if ! command -v docker &> /dev/null; then
    echo "[1/4] Installing Docker..."
    apt-get update
    apt-get install -y ca-certificates curl gnupg
    install -m 0755 -d /etc/apt/keyrings
    curl -fsSL https://download.docker.com/linux/ubuntu/gpg | gpg --dearmor -o /etc/apt/keyrings/docker.gpg
    chmod a+r /etc/apt/keyrings/docker.gpg
    echo "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/ubuntu $(. /etc/os-release && echo "$VERSION_CODENAME") stable" | tee /etc/apt/sources.list.d/docker.list > /dev/null
    apt-get update
    apt-get install -y docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin
    systemctl enable docker
    systemctl start docker
    echo "    Docker installed"
else
    echo "[1/4] Docker already installed"
fi

# 2. Create config directory
echo "[2/4] Creating Home Assistant config directory..."
mkdir -p /opt/homeassistant
echo "    Created /opt/homeassistant"

# 3. Run Home Assistant container
echo "[3/4] Starting Home Assistant container..."
docker run -d \
    --name homeassistant \
    --restart=unless-stopped \
    --privileged \
    -v /opt/homeassistant:/config \
    -v /run/dbus:/run/dbus:ro \
    -e TZ=America/New_York \
    --network=host \
    ghcr.io/home-assistant/home-assistant:stable

echo "    Home Assistant container started"

# 4. Wait for startup
echo "[4/4] Waiting for Home Assistant to initialize..."
sleep 10

# Check if running
if docker ps | grep -q homeassistant; then
    echo ""
    echo "=== Setup Complete ==="
    echo ""
    echo "Home Assistant is running!"
    echo ""
    echo "Access it at:"
    echo "  Local:    http://localhost:8123"
    echo "  Tailnet:  http://100.110.137.103:8123"
    echo ""
    echo "First-time setup:"
    echo "  1. Open the URL above"
    echo "  2. Create your admin account"
    echo "  3. Set up your home location"
    echo "  4. Home Assistant will auto-discover ESPHome devices"
    echo ""
    echo "Useful commands:"
    echo "  docker logs homeassistant -f     # View logs"
    echo "  docker restart homeassistant     # Restart"
    echo "  docker stop homeassistant        # Stop"
    echo "  docker start homeassistant       # Start"
    echo ""
else
    echo "ERROR: Home Assistant container failed to start"
    docker logs homeassistant
    exit 1
fi
