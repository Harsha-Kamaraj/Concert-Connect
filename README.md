# Concert Connect

A concert booking management system built in C that allows users to browse events, book seats, and manage reservations.

## Features

- **User Management**: Register and login for customers and admins
- **Event Management**: Admins can add, edit, and delete concert events
- **Seat Booking**: Customers can book available seats for events
- **Booking Management**: View and cancel bookings
- **Persistent Storage**: Data is saved to text files for persistence

## Project Structure

```
├── main.c          # Main application entry point
├── users.c/h       # User management (registration, login, authentication)
├── events.c/h      # Event management (add, edit, delete events)
├── bookings.c/h    # Booking system (book, cancel, view bookings)
├── utils.c/h       # Utility functions (input handling, UI helpers)
├── Makefile        # Build configuration
└── README.md       # This file
```

## Requirements

- GCC compiler (or any C99 compatible compiler)
- Make

## Building

To compile the project:

```bash
make
```

This will create an executable named `concert_booking`.

## Running

To run the application:

```bash
./concert_booking
```

## Usage

### First Run
On first run, you'll need to register users. The system supports two types of users:
- **Customer**: Can browse events and make bookings
- **Admin**: Can manage events and view all bookings

### Customer Features
- Browse available events
- Book seats for concerts
- View your bookings
- Cancel bookings

### Admin Features
- Add new events with details (name, date, venue, total seats)
- Edit existing events
- Delete events
- View all bookings across all events

## Data Persistence

The application stores data in text files:
- `users.txt`: User accounts and credentials
- `events.txt`: Concert events information
- `bookings.txt`: Booking records

These files are auto-generated on first save and are excluded from version control.

## Cleaning

To remove compiled files:

```bash
make clean
```

## License

This project is open source and available for educational purposes.
