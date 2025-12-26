#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "users.h"
#include "events.h"
#include "bookings.h"

static void customer_portal_flow(User *user) {
    while (1) {
        print_divider();
        printf("+============================================================+\n");
        printf("|       CUSTOMER PORTAL - %s\n", user->username);
        printf("+============================================================+\n");
        printf("1) Book seats\n2) Cancel seats\n3) View my bookings\n4) Exit\nChoose: ");
        int ch = read_int();
        if (ch == 1) {
            if (event_count == 0) {
                printf("No events available at the moment. Please check later.\n");
                pause_enter();
                continue;
            }
            list_events_brief();
            printf("Enter event number to book: ");
            int ev = read_int(); ev -= 1;
            if (ev < 0 || ev >= event_count) { printf("Invalid event.\n"); pause_enter(); continue; }
            int res = book_seat_for_user(ev, user);
            if (res == 1) {
                save_bookings_to_file("bookings.txt");
                save_events_to_file("events.txt");
            }
            (void)res;
            pause_enter();
        } else if (ch == 2) {
            if (event_count == 0) { printf("No events.\n"); pause_enter(); continue; }
            list_events_brief();
            printf("Enter event number to cancel booking from: ");
            int ev = read_int(); ev -= 1;
            if (ev < 0 || ev >= event_count) { printf("Invalid event.\n"); pause_enter(); continue; }
            int ok = cancel_seat_by_user(ev, user);
            if (!ok) printf("No cancellation performed.\n");
            else {
                save_bookings_to_file("bookings.txt");
                save_events_to_file("events.txt");
            }
            pause_enter();
        } else if (ch == 3) {
            view_my_bookings(user);
            pause_enter();
        } else if (ch == 4) {
            printf("Exiting customer portal.\n");
            break;
        } else {
            printf("Invalid option.\n");
        }
    }
}

static void admin_portal_flow(User *user) {
    while (1) {
        print_divider();
        printf("+============================================================+\n");
        printf("|       ADMIN PORTAL - %s\n", user->username);
        printf("+============================================================+\n");
        printf("1) List events\n");
        printf("2) Create event\n");
        printf("3) Delete event\n");
        printf("4) Display full seat map (all events)\n");
        printf("5) View all bookings (all events)\n");
        printf("6) View waiting queue (choose event)\n");
        printf("7) Booking Analytics\n");
        printf("8) Search bookings\n");
        printf("9) Cancel booking by ID\n");
        printf("10) View customer database\n");
        printf("11) Change ticket prices\n");
        printf("12) Exit\nChoose: ");
        int ch = read_int();
        if (ch == 1) {
            list_events_brief();
            pause_enter();
        } else if (ch == 2) {
            int created = create_event_interactive();
            if (created) save_events_to_file("events.txt");
            pause_enter();
        } else if (ch == 3) {
            delete_event_interactive();
            save_events_to_file("events.txt");
            save_bookings_to_file("bookings.txt");
            pause_enter();
        } else if (ch == 4) {
            show_full_seatmap_all_events();
            pause_enter();
        } else if (ch == 5) {
            show_all_bookings_admin();
            pause_enter();
        } else if (ch == 6) {
            if (event_count == 0) { printf("No events.\n"); pause_enter(); continue; }
            list_events_brief();
            printf("Enter event number to view its waiting queue: ");
            int ev = read_int(); ev -= 1;
            if (ev < 0 || ev >= event_count) { printf("Invalid event.\n"); continue; }
            show_waiting_queue(ev);
            pause_enter();
        } else if (ch == 7) {
            show_booking_analytics();
            pause_enter();
        } else if (ch == 8) {
            search_bookings_interactive();
            pause_enter();
        } else if (ch == 9) {
            printf("Enter Booking ID to cancel: ");
            char bid[32];
            read_line(bid, sizeof(bid));
            int cancelled = cancel_booking_by_id(bid);
            if (cancelled) {
                save_bookings_to_file("bookings.txt");
                save_events_to_file("events.txt");
            }
            pause_enter();
        } else if (ch == 10) {
            printf("\n+============================================================+\n");
            printf("|          CUSTOMER DATABASE                                 |\n");
            printf("+============================================================+\n");
            if (user_count == 0) { printf(" (no users registered)\n"); }
            for (int i = 0; i < user_count; ++i) {
                const char *role_str = (users[i].role == ROLE_ADMIN ? "ADMIN" : 
                                       (users[i].role == ROLE_CUSTOMER ? "CUSTOMER" : "NONE"));
                printf(" %d) %s | %s | phone: %s | email: %s\n", i + 1,
                    users[i].username, role_str, users[i].phone, users[i].email);
            }
            pause_enter();
        } else if (ch == 11) {
            change_ticket_price_interactive();
            save_events_to_file("events.txt");
            pause_enter();
        } else if (ch == 12) {
            printf("Exiting admin portal.\n");
            break;
        } else {
            printf("Invalid option.\n");
        }
    }
}

int main(void) {
    init_events_system();
    ensure_user_capacity();

    load_events_from_file("events.txt");

    load_bookings_from_file("bookings.txt");
    
    load_users_from_file("users.txt");

    printf("Welcome to Concert Booking System\n");
    print_divider();

    while (1) {
        printf("\nMain Menu:\n1) Customer Portal\n2) Admin Portal\n3) Exit\nChoose: ");
        int main_choice = read_int();
        if (main_choice == 1) {
            printf("\nCustomer Portal:\n1) Sign Up\n2) Sign In\nChoose: ");
            int cs = read_int();
            if (cs == 1) {
                int idx = signup_customer();
                if (idx >= 0) {
                    printf("Sign in now? (y/n): ");
                    char yn[8]; read_line(yn, sizeof(yn));
                    if (yn[0] == 'y' || yn[0] == 'Y') {
                        int idxx;
                        if (signin_customer(&idxx)) customer_portal_flow(&users[idxx]);
                    }
                }
            } else if (cs == 2) {
                int idx;
                if (signin_customer(&idx)) {
                    customer_portal_flow(&users[idx]);
                }
            } else {
                printf("Invalid choice.\n");
            }
        } else if (main_choice == 2) {
            printf("\nAdmin Portal:\n1) Sign Up\n2) Sign In\nChoose: ");
            int as = read_int();
            if (as == 1) {
                int idx = signup_admin();
                if (idx >= 0) {
                    printf("Sign in now? (y/n): ");
                    char yn[8]; read_line(yn, sizeof(yn));
                    if (yn[0] == 'y' || yn[0] == 'Y') {
                        int idxx;
                        if (signin_admin(&idxx)) admin_portal_flow(&users[idxx]);
                    }
                }
            } else if (as == 2) {
                int idx;
                if (signin_admin(&idx)) {
                    admin_portal_flow(&users[idx]);
                }
            } else {
                printf("Invalid choice.\n");
            }
        } else if (main_choice == 3) {
            printf("Exiting program. Goodbye.\n");
            /* Save all data before exiting */
            save_events_to_file("events.txt");
            save_bookings_to_file("bookings.txt");
            save_users_to_file("users.txt");
            break;
        } else {
            printf("Invalid option.\n");
        }
    }


    cleanup_events_system();
    free(users);
    return 0;
}