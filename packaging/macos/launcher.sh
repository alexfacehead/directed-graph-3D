#!/bin/bash
DIR="$(cd "$(dirname "$0")/../Resources" && pwd)"
xattr -cr "$DIR/directed_graph_bin" 2>/dev/null
exec "$DIR/directed_graph_bin"
