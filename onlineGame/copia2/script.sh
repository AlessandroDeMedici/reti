#!/bin/bash
make

# Avvia il server in una finestra separata
gnome-terminal --tab --title="Server" -- bash -c './server 1212; exec bash'

# Avvia il primo client in una finestra separata
gnome-terminal --tab --title="Client 1" -- bash -c './client 1212; exec bash'

# Avvia il secondo client in una finestra separata
gnome-terminal --tab --title="Client 2" -- bash -c './client 1212; exec bash'

