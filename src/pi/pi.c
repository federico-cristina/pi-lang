#include "pi/pi.h"

#ifndef PI_VERSION
/**
 * @brief   A string representing the current version of the Pi language.
 * 
 * @note    This symbolic constant is usually defined outside this file; the version shown
 *          below is therefore less specific than the one defined outside (for example, by
 *          CMake).
 * 
 *          It is kept only to cover the case where the version is not defined, but its
 *          existence is required (a practically impossible case, but covered).
 */
#   define PI_VERSION "0.7"
#endif

#include <stdio.h>

/* =---- Driver ------------------------------------------------= */

/** No errors: the execution was successful. */
#define PI_EXIT_SUCCESS 0
/** Error: Execution ended with a failed result. */
#define PI_EXIT_FAILURE 1
/** Error: Execution reached a point in the code that is invalid or not yet implemented. */
#define PI_EXIT_INVALID 2
/** Fatal error: execution did not reach completion, it was aborted. */
#define PI_EXIT_ABORTED 3

/**
 * @brief   This function runs the program in REPL (Read-Eval-Print-Loop) mode.
 * 
 * @return  This function returns an integer representing the final state of the program
 *          after its call.
 */
static int pi_Repl(void)
{
    /* Enable virtual terminal processing */
    piEnableVirtualTerminal();

    const char *const userProfile = getenv("UserProfile");

    /* Sets console title and current directory */
    piSetConsoleTitle("Pi " PI_VERSION);
    piSetCurrentDirectory(userProfile);

    bool
        /* This flag is raised during the execution of the main loop */
        keepRunning = true,
        /* This flag is raised when the error handling point is set */
        recoverySet = false;

    /* Prints initial message */
    puts(
        PI_GRAY "-- Welcome to Pi " PI_VERSION " REPL" PI_RESET "\n"
    );
    
    /* The output stream on which REPL messages will be written */
    FILE *const out = stdout;
    /* The input stream of source code from where REPL will read */
    PiSource in;

    /* Opens stdin as a source code stream */
    piOpenReplStream(&in, "<stdin>", stdin, /* maxLineLength: */ BUFSIZ);

    if (!recoverySet)
    {
        /* The error handling point is set */
        recoverySet = true;
        
        jmp_buf replJmpBuf;

        /* Sets the repl error handler onto handlers stack */
        piSetErrorHandler(&replJmpBuf);

        /* Recovery code */
        if (setjmp(replJmpBuf) != 0)
        {
            /* Notifies the pending execution abortion */
            puts("\nAn error occurred: " PI_ErroneousColor("Aborting") "...");

            return PI_EXIT_ABORTED;
        }
    }

    do
    {
        /* Prints default prompt */
        fputs(">>> ", out);

        /* Reads a line from REPL stream handling interrupts */
        if (piReplReadLine(&in) < 0)
            break;

        /*
        
        TODO: Eval and Print phases of REPL

         */

        PiToken token;

        do
        {
            piScanToken(&in, &token);

            if (token.code == PI_TOKEN_ENDOF)
                break;

            fprintf(out, "[%d, %d] '%.*s'\n", token.line, token.column, (int)token.length, token.text);
        } while (true);
    } while (keepRunning);

    /* Closes the source code stream */
    piCloseSource(&in);

    return PI_EXIT_SUCCESS;
}

/**
 * @brief   This function runs the program in 'command line' mode, interpreting
 *          and executing the specified command line arguments.
 * 
 * @param     argc  The number of arguments passed per command line (i.e., the number of
 *                  elements inside `argv`).
 * @param[in] argv  An array of strings representing the tokens passed as command line
 *                  arguments.
 * 
 * @return  This function returns an integer representing the final state of the program
 *          after its call.
 */
static int pi_Main(const int argc, const char *const argv[])
{
    /* Not implemented yet! */
    return PI_EXIT_INVALID;
}

/**
 * +---- EntryPoint -----------------------+
 */

int main(const int argc, const char *const argv[])
{
    int exitCode;
    
    /* Configures the system before execution */
    piInitSystem();

    PiEnv env;

    piInitEnv(&env);

    void *block = piEnvAlloc(&env, 8000);

    piEnvFree(&env, block);

    block = piStringNew(&env, "Hello, world!", 13);

    PiEnv child;

    piCreateChildEnv(&env, &child, PI_CAP_NONE, 0);
    piCapabilityContextSetLimits(&child.caps, 0, 0, 0);

    block = piEnvAlloc(&child, 8000);

    piFreeEnv(&env);

    if (argc < 2)
        exitCode = pi_Repl();
    else
        exitCode = pi_Main(argc, argv);

    /* Resets system configuration after execution */
    piFreeSystem();

    return exitCode;
}

/* =------------------------------------------------------------= */
