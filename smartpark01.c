
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define SLOTS 15
#define PASSES 20

#define TWO_WHEELER 1
#define FOUR_WHEELER 2

#define TWO_RATE 10.0
#define FOUR_RATE 20.0
#define DISCOUNT 15.0

struct Vehicle {
    char number[20];
    int type;
    int ticket;
    int passId;
    time_t entry;
    int parked;
};

struct Pass {
    int id;
    char name[30];
    float balance;
    time_t startDate;
    time_t expiryDate;
};

struct Vehicle v[SLOTS];
struct Pass p[PASSES];

int passCount = 0;
int nextTicket = 1001;
int nextPass = 5001;
int totalVehicles = 0;
float revenue = 0;

/* Convert vehicle number to uppercase */
void upper(char str[]) {
    int i;

    for (i = 0; str[i] != '\0'; i++)
        str[i] = toupper((unsigned char)str[i]);
}

/* Validate Indian vehicle number format */
int validateVehicle(char number[]) {
    int i;

    if (strlen(number) != 10)
        return 0;

    for (i = 0; i < 10; i++) {
        if (i < 2 || (i >= 4 && i < 6)) {
            if (!isalpha((unsigned char)number[i]))
                return 0;
        } else {
            if (!isdigit((unsigned char)number[i]))
                return 0;
        }
    }

    return 1;
}

/* Find a parked vehicle */
int findVehicle(char number[]) {
    int i;

    for (i = 0; i < SLOTS; i++) {
        if (v[i].parked &&
            strcmp(v[i].number, number) == 0)
            return i;
    }

    return -1;
}

/* Find a parking pass */
int findPass(int id) {
    int i;

    for (i = 0; i < passCount; i++) {
        if (p[i].id == id)
            return i;
    }

    return -1;
}

/* Check whether the monthly pass is valid */
int validPass(int index) {
    time_t now = time(NULL);

    if (index == -1)
        return 0;

    return now < p[index].expiryDate;
}

/* Allocate parking slot */
int findSlot(int type) {
    int i, start, end;

    if (type == TWO_WHEELER) {
        start = 0;
        end = 4;
    } else {
        start = 5;
        end = 14;
    }

    for (i = start; i <= end; i++) {
        if (!v[i].parked)
            return i;
    }

    /* Two-wheelers can use slots 6-15 */
    if (type == TWO_WHEELER) {
        for (i = 5; i < SLOTS; i++) {
            if (!v[i].parked)
                return i;
        }
    }

    return -1;
}

/* Vehicle entry */
void entry() {
    char number[20], choice;
    int type, slot, id, passIndex = -1;

    printf("\nEnter vehicle number: ");
    scanf("%19s", number);

    upper(number);

    if (!validateVehicle(number)) {
        printf("Invalid vehicle number!\n");
        printf("Example: KA03AB1234\n");
        return;
    }

    if (findVehicle(number) != -1) {
        printf("Vehicle is already parked!\n");
        return;
    }

    printf("1. Two-wheeler\n");
    printf("2. Four-wheeler\n");
    printf("Enter vehicle type: ");
    scanf("%d", &type);

    if (type != TWO_WHEELER &&
        type != FOUR_WHEELER) {
        printf("Invalid vehicle type!\n");
        return;
    }

    printf("Do you have a monthly parking pass? (y/n): ");
    scanf(" %c", &choice);

    if (choice == 'y' || choice == 'Y') {
        printf("Enter pass ID: ");
        scanf("%d", &id);

        passIndex = findPass(id);

        if (passIndex == -1) {
            printf("Pass not found!\n");
            return;
        }

        if (p[passIndex].balance < 50) {
            printf("Please recharge your pass first!\n");
            return;
        }

        if (validPass(passIndex))
            printf("Monthly pass is valid. 15%% discount available.\n");
        else
            printf("Monthly pass expired. Normal rates apply.\n");
    }

    slot = findSlot(type);

    if (slot == -1) {
        printf("No parking slot available!\n");
        return;
    }

    strcpy(v[slot].number, number);
    v[slot].type = type;
    v[slot].ticket = nextTicket++;
    v[slot].entry = time(NULL);
    v[slot].parked = 1;

    if (passIndex == -1)
        v[slot].passId = 0;
    else
        v[slot].passId = p[passIndex].id;

    totalVehicles++;

    printf("\n--- PARKING TICKET ---\n");
    printf("Ticket number: T%d\n", v[slot].ticket);
    printf("Vehicle number: %s\n", number);
    printf("Vehicle type: %s\n",
           type == TWO_WHEELER ?
           "Two-wheeler" : "Four-wheeler");
    printf("Parking slot: %d\n", slot + 1);
    printf("Entry time: %s", ctime(&v[slot].entry));
    printf("Payment: %s\n",
           passIndex == -1 ? "Cash" : "Monthly Pass");
}

/* Vehicle exit and fee calculation */
void exitVehicle() {
    char number[20];
    int slot, hours, passIndex;
    double seconds;
    float rate, fee, discountAmount = 0;
    float passPayment = 0, cashPayment = 0;

    printf("\nEnter vehicle number: ");
    scanf("%19s", number);

    upper(number);

    if (!validateVehicle(number)) {
        printf("Invalid vehicle number!\n");
        printf("Example: KA03AB1234\n");
        return;
    }

    slot = findVehicle(number);

    if (slot == -1) {
        printf("Vehicle not found!\n");
        return;
    }

    seconds = difftime(time(NULL), v[slot].entry);

    /* Demo mode: one minute counts as one hour */
    hours = (int)(seconds / 60);

    if (seconds > hours * 60)
        hours++;

    if (hours < 1)
        hours = 1;

    if (v[slot].type == TWO_WHEELER)
        rate = TWO_RATE;
    else
        rate = FOUR_RATE;

    /* Calculate normal parking fee */
    fee = hours * rate;

    /* Find the pass used during entry */
    passIndex = findPass(v[slot].passId);

    /* Apply 15% discount for a valid monthly pass */
    if (validPass(passIndex)) {
        discountAmount = fee * DISCOUNT / 100;
        fee = fee - discountAmount;
    }

    printf("\n--- PARKING RECEIPT ---\n");
    printf("Ticket number: T%d\n", v[slot].ticket);
    printf("Vehicle number: %s\n", v[slot].number);
    printf("Parking slot: %d\n", slot + 1);
    printf("Parking duration: %d hour(s)\n", hours);
    printf("Normal fee: Rs. %.2f\n", hours * rate);

    if (discountAmount > 0) {
        printf("Monthly pass discount: Rs. %.2f\n",
               discountAmount);
    } else if (passIndex != -1) {
        printf("Monthly pass expired. No discount applied.\n");
    }

    printf("Final parking fee: Rs. %.2f\n", fee);

    /* Deduct from pass balance first */
    if (passIndex != -1) {
        passPayment = p[passIndex].balance;

        if (passPayment > fee)
            passPayment = fee;

        p[passIndex].balance -= passPayment;
        cashPayment = fee - passPayment;
    } else {
        cashPayment = fee;
    }

    printf("Paid by pass: Rs. %.2f\n", passPayment);
    printf("Remaining cash payment: Rs. %.2f\n", cashPayment);

    if (passIndex != -1)
        printf("Pass balance remaining: Rs. %.2f\n",
               p[passIndex].balance);

    revenue += fee;

    v[slot].parked = 0;
    v[slot].number[0] = '\0';
    v[slot].passId = 0;

    printf("Vehicle exited successfully!\n");
}

/* Search vehicle */
void searchVehicle() {
    char number[20];
    int slot;

    printf("Enter vehicle number: ");
    scanf("%19s", number);

    upper(number);

    if (!validateVehicle(number)) {
        printf("Invalid vehicle number!\n");
        printf("Example: KA03AB1234\n");
        return;
    }

    slot = findVehicle(number);

    if (slot == -1) {
        printf("Vehicle is not parked.\n");
    } else {
        printf("\nVehicle found!\n");
        printf("Ticket: T%d\n", v[slot].ticket);
        printf("Vehicle: %s\n", v[slot].number);
        printf("Slot: %d\n", slot + 1);
        printf("Type: %s\n",
               v[slot].type == TWO_WHEELER ?
               "Two-wheeler" : "Four-wheeler");
        printf("Entry time: %s", ctime(&v[slot].entry));
    }
}

/* Show live parking slots */
void showSlots() {
    int i;

    printf("\n--- PARKING SLOT STATUS ---\n");

    for (i = 0; i < SLOTS; i++) {
        if (v[i].parked)
            printf("Slot %d: Occupied by %s\n",
                   i + 1, v[i].number);
        else
            printf("Slot %d: Available\n", i + 1);
    }
}

/* Parking dashboard */
void dashboard() {
    int i, parked = 0;

    for (i = 0; i < SLOTS; i++) {
        if (v[i].parked)
            parked++;
    }

    printf("\n--- PARKING DASHBOARD ---\n");
    printf("Total slots: %d\n", SLOTS);
    printf("Total vehicles entered: %d\n", totalVehicles);
    printf("Currently parked: %d\n", parked);
    printf("Available slots: %d\n", SLOTS - parked);
    printf("Occupancy: %.1f%%\n",
           parked * 100.0 / SLOTS);
    printf("Total revenue: Rs. %.2f\n", revenue);
    printf("Monthly passes issued: %d\n", passCount);
}

/* Issue monthly parking pass */
void issuePass() {
    float amount;
    time_t now;

    if (passCount >= PASSES) {
        printf("Cannot issue more passes!\n");
        return;
    }

    printf("Enter pass holder name: ");
    scanf(" %29[^\n]", p[passCount].name);

    printf("Enter initial balance (minimum Rs. 50): ");
    scanf("%f", &amount);

    if (amount < 50) {
        printf("Amount must be at least Rs. 50.\n");
        return;
    }

    now = time(NULL);

    p[passCount].id = nextPass++;
    p[passCount].balance = amount;
    p[passCount].startDate = now;

    /* Monthly pass valid for 30 days */
    p[passCount].expiryDate =
        now + (30 * 24 * 60 * 60);

    printf("\n--- MONTHLY PARKING PASS ---\n");
    printf("Pass ID: %d\n", p[passCount].id);
    printf("Holder: %s\n", p[passCount].name);
    printf("Balance: Rs. %.2f\n", p[passCount].balance);
    printf("Discount: 15%%\n");
    printf("Start date: %s", ctime(&p[passCount].startDate));
    printf("Expiry date: %s", ctime(&p[passCount].expiryDate));

    passCount++;
}

/* Recharge parking pass */
void rechargePass() {
    int id, index;
    float amount;

    printf("Enter pass ID: ");
    scanf("%d", &id);

    index = findPass(id);

    if (index == -1) {
        printf("Pass not found!\n");
        return;
    }

    printf("Enter recharge amount: ");
    scanf("%f", &amount);

    if (amount <= 0) {
        printf("Invalid amount!\n");
        return;
    }

    p[index].balance += amount;

    printf("Recharge successful!\n");
    printf("New balance: Rs. %.2f\n", p[index].balance);

    if (!validPass(index))
        printf("Pass has expired. Issue a new pass for the discount.\n");
}

/* Check pass balance and expiry */
void checkBalance() {
    int id, index;

    printf("Enter pass ID: ");
    scanf("%d", &id);

    index = findPass(id);

    if (index == -1) {
        printf("Pass not found!\n");
    } else {
        printf("\n--- MONTHLY PASS DETAILS ---\n");
        printf("Pass ID: %d\n", p[index].id);
        printf("Holder: %s\n", p[index].name);
        printf("Balance: Rs. %.2f\n", p[index].balance);
        printf("Start date: %s", ctime(&p[index].startDate));
        printf("Expiry date: %s", ctime(&p[index].expiryDate));

        if (validPass(index))
            printf("Status: Active (15%% discount)\n");
        else
            printf("Status: Expired\n");
    }
}

/* Main function - unchanged */
int main() {
    int choice, i;

    for (i = 0; i < SLOTS; i++)
        v[i].parked = 0;

    do {
        printf("\n===== SMARTPARK SYSTEM =====\n");
        printf("1. Vehicle Entry\n");
        printf("2. Vehicle Exit\n");
        printf("3. Search Vehicle\n");
        printf("4. Show Parking Slots\n");
        printf("5. Parking Dashboard\n");
        printf("6. Issue Parking Pass\n");
        printf("7. Recharge Parking Pass\n");
        printf("8. Check Pass Balance\n");
        printf("0. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: entry(); break;
            case 2: exitVehicle(); break;
            case 3: searchVehicle(); break;
            case 4: showSlots(); break;
            case 5: dashboard(); break;
            case 6: issuePass(); break;
            case 7: rechargePass(); break;
            case 8: checkBalance(); break;
            case 0:
                printf("Thank you for using SmartPark!\n");
                break;
            default:
                printf("Invalid choice!\n");
        }
    } while (choice != 0);

    return 0;
}
