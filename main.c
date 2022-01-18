// Lunar Descent Program
// Labb #1 | HI1024 | TIMEL2021
// Sinan Pasic


#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

struct simulation {
    double height;
    double velocity;
    double fuel;
    int time;
} simdata;

static struct simulation const default_sim = { 250.0, -25.0, 500.0, 0 };

void         sim_init           (struct simulation *simp);
double       velocity_step      (double throttle);
void         sim_printstate     (struct simulation *simp);
void         sim_printheader    (void);
double       sim_sanitize       (struct simulation *simp, double value);
char const * sim_get_quitreason (struct simulation const *simp);
int          sim_step           (struct simulation *simp);
char const * get_first_digit    (char const *buffer, size_t bufsz);
size_t       strncpy_digits     (char *dest, char const *buffer, size_t bufsz);
uint64_t     strtou64           (char const *buffer, size_t bufsz);
double       strtodouble        (char const *buffer, size_t bufsz);
double       read_float         (char const *prompt);
int          main               (void);

void
sim_init(struct simulation *simp)
{
    memcpy(simp, &default_sim, sizeof default_sim);
}

double
velocity_step(double throttle)
{
    return (throttle / 10) - 1.5;
}

void
sim_printstate(struct simulation *simp)
{
    printf("%4u %6.2lf %8.2lf %6.2lf ", simp->time, simp->height, simp->velocity, simp->fuel);
}

void
sim_printheader(void)
{
    puts("Time Height Velocity Fuel   Throttle?");
}

// 1. It makes more sense to clamp than to quit or re-prompt
double
sim_sanitize(struct simulation *simp, double value)
{
    // 1
    if (value < 0.0)
        return 0.0;

    if (value > 100.0)
        return 100.0;

    return value;
}

char const *
sim_get_quitreason(struct simulation const *simp)
{
    if(simp->fuel <= 0)
        return "Out of fuel";

    if(feof(stdin))
        return "The player quit";

    return "A wizard zapped the lander idk";
}

int
sim_step(struct simulation *simp)
{
    static char const *promptstr = "throttle: ";
    double throttle = 0;

    if (simp->height <= 0.0) {
        if(simp->velocity < -2.0)
            goto exit_lose;

        goto exit_win;
    }

    if(simp->fuel > 0 && !feof(stdin))
        throttle = sim_sanitize(simp, read_float(promptstr));
    else
        printf("%s0.0 (%s)\n", promptstr, sim_get_quitreason(simp));

    simp->time += 1;
    simp->velocity += velocity_step(throttle);
    simp->height = simp->height + simp->velocity - velocity_step(throttle) / 2;
    simp->fuel -= throttle;

    if(simp->fuel < 0)
        simp->fuel = 0;

    sim_printstate(simp);
    return 1;

exit_lose:
    {
        puts("\nYou crashed.");
        return 0;
    }

exit_win:
    {
        puts("\nSuccessfully landed.");
        return 0;
    }
}

char const *
get_first_digit(char const *buffer, size_t bufsz)
{
    for (size_t buflen = 0; buflen < bufsz; ++buflen)
        if (isdigit(buffer[buflen]))
            return buffer + buflen;

    return NULL;
}

size_t
strncpy_digits(char *dest, char const *buffer, size_t bufsz)
{
    size_t len = 0;

    while (len < bufsz && isdigit(buffer[len])) {
        dest[len] = buffer[len];
        ++len;
    }

    return len;
}

uint64_t
strtou64(char const *buffer, size_t bufsz)
{
    uint64_t out = 0;

    for (size_t i = 0; i < bufsz; ++i)
        out = 10 * out + (buffer[i] - '0');

    return out;
}

#define DIGIT_BUFFER_SETUP() do { \
    size_t dbuflen; \
    char digit_buffer[32]; \
    uint64_t digit_value_u64; \
    bufsz -= (match - buffer); \
    buffer = match; \
    dbuflen = strncpy_digits(digit_buffer, buffer, bufsz); \
    digit_value_u64 = strtou64(digit_buffer, dbuflen); \
    digit_value = (double)digit_value_u64; \
} while(0)

double
strtodouble(char const *buffer, size_t bufsz)
{
    double accum;
    double sign = 1.0;
    double digit_value;
    char const *match;

    match = memchr(buffer, '-', bufsz);

    if (match != NULL) {
        sign = -1.0;
        ++match;
        bufsz -= (match - buffer);
        buffer = match;
    }

    match = get_first_digit(buffer, bufsz);

    if (match == NULL)
        return 0.0;

    DIGIT_BUFFER_SETUP();
    match = memchr(buffer, '.', bufsz);

    if (match == NULL)
        return digit_value * sign;

    ++match;
    accum = digit_value;
    DIGIT_BUFFER_SETUP();

    while (digit_value >= 1.0)
        digit_value /= 10;

    return (accum + digit_value) * sign;
}

#undef DIGIT_BUFFER_SETUP

double
read_float(char const *prompt)
{
    char buffer[32];
    size_t buflen = 0;
    int ch;

    fputs(prompt, stdout);

    while ((ch = getchar()) != EOF && ch != '\n')
        if (buflen < 32)
            buffer[buflen++] = ch;

    if (ch == EOF)
        putchar('\n');

    return strtodouble(buffer, buflen);
}

int
main(void)
{
    struct simulation sim;

    sim_init(&sim);
    puts("For each step, enter a throttle value (0..100)");
    sim_printheader();
    sim_printstate(&sim);

    while (sim_step(&sim))
        ;

    return 0;
}
