# Algendado - Personal Agenda Server/Client

A simple C-based personal agenda application with server/client architecture.

## Features

- Add agenda items with date/time and descriptions
- List items by day, week, or month
- Beautiful visual popup notifications (raylib)
- Desktop notifications (macOS)
- Web interface with calendar view
- ICS calendar export
- SQLite database storage

## Installation

### Prerequisites (macOS)

```bash
brew install sqlite libmicrohttpd raylib
```

### Build

```bash
make
```

### Install (optional)

```bash
make install
```

## Usage

### Adding items

```bash
# Add item for today
./algen add today 11:15:00 "finish the project"

# Add item for tomorrow
./algen add tomorrow 14:30:00 "meeting with team"

# Add item for specific date
./algen add 15/07/2025 09:00:00 "doctor appointment"
```

### Listing items

```bash
# List today's items
./algen get today

# List this week's items
./algen get week

# List this month's items
./algen get month
```

### Server

The server starts automatically when adding items. You can also start it manually:

```bash
./algen-server
```

Access the web interface at: http://localhost:8080
Access the ICS calendar at: http://localhost:8080/calendar.ics

## Project Structure

```
src/
├── client.c        # Main client application
├── server.c        # Server application
├── database.c      # Database operations
├── notifications.c # Desktop notifications
├── web_handler.c   # Web interface handler
└── calendar.c      # Calendar and ICS export

include/
└── agenda.h        # Common headers and structures
```
