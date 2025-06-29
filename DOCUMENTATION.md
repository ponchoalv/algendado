# Algendado - Personal Agenda Application

A complete C-based personal agenda application with server/client architecture, featuring SQLite database storage, desktop notifications, web interface, and ICS calendar export.

## 🌟 Features

- ✅ **Command-line interface** for adding and viewing agenda items
- ✅ **Automatic server startup** when adding items
- ✅ **SQLite database** for persistent storage
- 🆕 **Beautiful visual notifications** with raylib - attractive popup windows
- ✅ **Desktop notifications** (macOS) - system notifications as backup
- ✅ **Web interface** with beautiful calendar view
- ✅ **ICS calendar export** for integration with any calendar client
- ✅ **Flexible date formats** (today, tomorrow, DD/MM/YYYY)
- ✅ **Multiple view periods** (today, week, month)

## 🛠 Installation

### Prerequisites (macOS)

```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies (including raylib for visual notifications)
brew install sqlite libmicrohttpd raylib
```

### Quick Install

```bash
# Clone and build
git clone <your-repo-url> algendado
cd algendado
chmod +x install.sh
./install.sh
```

### Manual Build

```bash
# Build the project
make

# Optional: Install to system
make install
```

## 📖 Usage

### Adding Agenda Items

```bash
# Add item for today
./algen add today 11:15:00 "finish the project"

# Add item for tomorrow
./algen add tomorrow 14:30 "meeting with team"

# Add item for specific date
./algen add 15/07/2025 09:00:00 "doctor appointment"

# Times can be HH:MM or HH:MM:SS
./algen add today 16:45 "quick meeting"
```

### Viewing Agenda Items

```bash
# View today's items
./algen get today

# View this week's items
./algen get week

# View this month's items
./algen get month
```

### Web Interface

Once you add your first item, the server starts automatically. Access:

- **Web Calendar**: http://localhost:8080
- **ICS Export**: http://localhost:8080/calendar.ics

### Manual Server Control

```bash
# Start server manually
./algen-server

# Stop server (Ctrl+C)
```

## 🔔 Visual & Desktop Notifications

The application features **dual notification system**:

### 🎨 Visual Notifications (NEW!)
- **Beautiful raylib-powered popup windows** that appear 15 minutes before events
- **Smooth slide-in animations** with modern styling
- **Interactive interface** - click OK or press ESC to dismiss
- **Auto-dismiss** after 30 seconds
- **Rich content display** with emoji icons, formatted time, and message wrapping
- **Elegant design** with gradient colors and subtle glow effects

### 🍎 System Notifications (Backup)
- Standard macOS notifications as fallback
- Includes event title, time, and description
- Plays system sound

Both notification types include:
- Event title
- Scheduled time
- Description

## 📅 Calendar Integration

Download the ICS calendar file and import it into:
- **Apple Calendar**: File → Import
- **Google Calendar**: Settings → Import & Export
- **Outlook**: File → Open & Export → Import/Export

Or subscribe to the live calendar using: `http://localhost:8080/calendar.ics`

## 🏗 Project Structure

```
algendado/
├── src/
│   ├── client.c        # Main client application
│   ├── server.c        # Server application
│   ├── database.c      # SQLite database operations
│   ├── notifications.c # Desktop notifications (macOS)
│   ├── web_handler.c   # HTTP request handling
│   ├── calendar.c      # Calendar HTML and ICS generation
│   └── utils.c         # Utility functions
├── include/
│   └── agenda.h        # Common headers and structures
├── build/              # Build artifacts (created during build)
├── Makefile           # Build configuration
├── install.sh         # Installation script
├── test.sh           # Test script
└── README.md         # This file
```

## 🧪 Testing

Run the test suite:

```bash
./test.sh
```

This will:
1. Build the project
2. Test adding items
3. Test retrieving items
4. Test web interface
5. Test ICS calendar export

## 🔧 Development

### Build Commands

```bash
# Clean build
make clean

# Build both client and server
make

# Build dependencies info
make install-deps
```

### Database

The application uses SQLite with the following schema:

```sql
CREATE TABLE agenda_items (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    date TEXT NOT NULL,           -- YYYY-MM-DD format
    time TEXT NOT NULL,           -- HH:MM:SS format
    description TEXT NOT NULL,
    datetime INTEGER NOT NULL,    -- Unix timestamp
    notified INTEGER DEFAULT 0    -- Notification status
);
```

### Configuration

Edit `include/agenda.h` to modify:
- `SERVER_PORT` (default: 8080)
- `DB_PATH` (default: agenda.db)
- `NOTIFICATION_ADVANCE_MINUTES` (default: 15)

## 🐛 Troubleshooting

### Build Issues

```bash
# If you get "microhttpd.h not found"
brew reinstall libmicrohttpd

# If you get linking errors
brew reinstall sqlite
```

### Runtime Issues

```bash
# If server won't start (port in use)
lsof -ti:8080 | xargs kill -9

# If database issues
rm agenda.db
```

### Notification Issues

- Ensure macOS notifications are enabled for Terminal
- System Preferences → Notifications & Focus → Terminal → Allow Notifications

## 📝 Examples

### Daily Workflow

```bash
# Morning planning
./algen add today 09:00 "Daily standup"
./algen add today 11:00 "Code review session"
./algen add today 15:30 "Client presentation"

# Check today's schedule
./algen get today

# View in browser
open http://localhost:8080
```

### Weekly Planning

```bash
# Plan the week
./algen add tomorrow 10:00 "Project kickoff"
./algen add 03/07/2025 14:00 "Sprint planning"
./algen add 04/07/2025 16:00 "Demo preparation"

# Review weekly schedule
./algen get week
```

## 🔮 Future Enhancements

Potential features for future versions:
- [ ] Event editing and deletion
- [ ] Recurring events
- [ ] Multiple calendar support
- [ ] Email notifications
- [ ] Linux/Windows notification support
- [ ] Mobile app integration
- [ ] Event categories and colors
- [ ] Search functionality
- [ ] Backup and sync features

## 📄 License

This project is released under the MIT License. See LICENSE file for details.

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

---

**Made with ❤️ in C** | Perfect for developers who love command-line tools and want a simple, efficient personal agenda system!
