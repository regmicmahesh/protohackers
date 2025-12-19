#!/bin/bash
set -euxo pipefail

cmake --build build

ssh -i ssh.key ubuntu@132.145.210.130 "pkill udp-database || true"

scp -i ssh.key ./build/bin/udp-database ubuntu@132.145.210.130:/home/ubuntu/mahesh/udp-database

ssh -i ssh.key ubuntu@132.145.210.130