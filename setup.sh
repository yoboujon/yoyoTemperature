#!/bin/bash

ENV_FILE=".env"

ask_string() {
    local prompt="$1"
    local hide="$2"
    local value
    local flags="-rp"

    if [[ "$hide" == true ]]; then
        flags="-rsp"
    fi

    while true; do
        read $flags "$prompt: " value
        if [[ -n "$value" ]]; then
            [[ "$hide" == true ]] && echo >&2
            echo "$value"
            return
        fi
        echo "Empty string." >&2
    done
}

ask_port() {
    local prompt="$1"
    local default="$2"
    local value

    while true; do
        read -rp "$prompt (default: $default): " value
        [[ -z "$value" ]] && value="$default"
        if [[ "$value" =~ ^[0-9]+$ ]] && (( "$value" >= 1 && "$value" <= 65535 )) ; then
            echo "$value"
            return
        fi
        echo "Invalid port: must be a number between 1 and 65535." >&2
    done
}

if [[ -f "$ENV_FILE" ]]; then
    read -rp ".env already exists. Overwrite? [y/n] " confirm
    if [[ ! "$confirm" =~ ^[Yy]$ ]]; then
        echo "Aborted."
        exit 1
    fi
fi

udp_port=$(ask_port "UDP Port" 5000)
api_ip=$(ask_string "API IP Address: " false)
api_port=$(ask_port "API Port" 8080)
wifi_ssid=$(ask_string "Wi-Fi SSID: " false)
wifi_password=$(ask_string "Wi-Fi Password: " true)
read -rp "Infoclimat token (unused if empty): " infoclimat_token

cat > "$ENV_FILE" <<EOF
UDP_PORT=${udp_port}
API_PORT=${api_port}
API_IP=${api_ip}
WIFI_SSID=${wifi_ssid}
WIFI_PASSWORD=${wifi_password}
EOF
if [[ -n "$infoclimat_token" ]]; then
    echo "INFOCLIMAT_TOKEN=${infoclimat_token}" >> "$ENV_FILE"
fi

chmod 600 "$ENV_FILE"

echo "Saved to $ENV_FILE"