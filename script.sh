#!/usr/bin/env bash
cxx() {
    echo =====================
    ./cxx.out "$@"
}

cxx <<EOF
heredoc contents
EOF

cxx <<< herestring some arg

echo from pipe | cxx

cat << EOF1 << EOF2 
hi
EOF1
there
EOF2
