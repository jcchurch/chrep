#include <string.h>
#include <stdlib.h>

#ifndef __JAMES_REGEX
#define __JAMES_REGEX

#define LAZY      1 // Lazy Backtracking
#define GREEDY    2 // Standard Backtracking
#define POSESSIVE 3 // No Backtracking

#define NO_MATCH -1
#define INFINITY -1

// The only one that anyone should be using.
int match(char *subject, char *regex, int *point, int *len);

void printStr(char *str, int start, int len);
char getEscapeChar (char c);
int  characterClass(char *regex, char c);
int  matchPosition(char *subject, char *regex, int regexLen);
int  matchSubset(char *subject, char *regex, int len);
int  matchAtom(char *subject, char *regex, int len, int min, int max, int *desired);

/* printStr
 * Pre: A string, a starting position, and a length
 * Post: Nothing, but the word will be printed on the screen.
 * This is used mostly for debugging.
 */
void printStr(char *str, int start, int len) {
    int i;
    for (i = start; i < start+len; i++) {
        printf("%c", str[i]);
    }
}

/* getEscapeChar
 * Pre: A character
 * Post The escape sequence for that character
 *      (or the same character if it has no
 *      special escape sequence).
 */
char getEscapeChar (char c) {

    switch (c) {
        case '0' : return 0;
        case 'a' : return 7;
        case 'b' : return 8;
        case 't' : return 9;
        case 'n' : return 10;
        case 'v' : return 11;
        case 'f' : return 12;
        case 'r' : return 13;
        case '\\': return 92;
    } // End Switch

    return c;
} // End get_esacpe_char

/* characterClass
 * Pre: A Character Class Expression, and a character to match
 * Post: A boolean of 1 (true) or 0 (false) if the expression
 *       was successful for not.
 *       A legal character class begins with a '[' and ends with ']'
 *       If the first symbol after the '[' is a '^', then the class
 *       is negated.
 */
int characterClass(char *regex, char c) {

    int i = 0;
    char p, q;
    int escaped;
    int success = 1;

    if (regex[i] == '[')
        i++;

    if (regex[i] == '^') {
        i++;
        success = 0;
    }

    while (1) {
        if (regex[i] == '\0' || regex[i] == ']') break;

        if (regex[i] == '\\') {
            p = getEscapeChar(regex[i+1]);
            i += 2;
        }
        else {
            p = regex[i];
            i++;
        }

        if (regex[i] == '-' && regex[i+1] != ']') {
            i++;
            if (regex[i] == '\\') {
                q = getEscapeChar(regex[i+1]);
                i += 2;
            }
            else {
                q = regex[i];
                i++;
            }

            if (p <= c && c <= q)
                return success;
        }
        else {
            if (p == c)
                return success;
        }
    }

    return 1-success;
}

/* match - Test a string for a regular expression
 * Pre: A string subject, A string regular expression,
 *      and two ints which pass data back out.
 * Post: 0 (false) or 1 (true) boolean success.
 *       The 'point' variable will record the location
 *       of the successful match (or -1 on failure).
 *       The 'len' variable will record the length
 *       of the successful match (or -1 on failure).
 */
int match(char *subject, char *regex, int *point, int *len) {

    int matchLen;
    int regexLen = strlen(regex);
    int subjectLen = strlen(subject);
    int mustMatchEnd = 0;
    int begin = 0;
    int insideClass = 0;
    int parens = 0;
    int i = 0;

    *point = -1;
    *len   = -1;
    if (regex[regexLen-1] == '$') {
        regexLen--;
        mustMatchEnd = 1;
    }

    if (regex[0] == '^') {
        begin = 1;
    }

    i = begin; // i now acts as a regex index
    while (i < regexLen) {

        if (regex[i] == '{') {
            if (regex[i+1] == '}') {
                fprintf(stderr, "ERROR: regex /%s/ has empty '{}' modifier brace.\n", regex);
                exit(1); 
            }

            while (i < regexLen) {
                i++;
                if (regex[i] == '}') break;

                if (i >= regexLen) {
                    fprintf(stderr, "ERROR: regex /%s/ has '{' brace but no ending brace.\n", regex, regex[i]);
                    exit(1); 
                }

                if (regex[i] != ',' && ('0' > regex[i] || regex[i] > '9')) {
                    fprintf(stderr, "ERROR: regex /%s/ has illegal character '%c' in '{}' brace.\n", regex, regex[i]);
                    exit(1); 
                }
            }
        }

        if (regex[i] == '[' && insideClass == 0) {
            if (regex[i+1] == ']') {
                fprintf(stderr, "ERROR: regex /%s/ has empty '[]' character class.\n", regex);
                exit(1); 
            }
            else {
                insideClass = 1;
            }
        }

        if (regex[i] == ']') insideClass = 0;

        if (insideClass == 0) {
            if (regex[i] == ')') parens--;
            if (regex[i] == '(') parens++;
        }

        if (regex[i] == '\\') i++;
        i++;
    }

    if (parens > 0) {
        fprintf(stderr, "ERROR: regex /%s/ failed due %d unmatched '(' symbols.\n", regex, parens);
        exit(1);
    }

    if (parens < 0) {
        fprintf(stderr, "ERROR: regex /%s/ failed due %d unmatched ')' symbols.\n", regex, -1*parens);
        exit(1);
    }

    if (insideClass == 1) {
        fprintf(stderr, "ERROR: regex /%s/ failed due to unmatched '['.\n", regex);
        exit(1);
    }

    if (i > regexLen) {
        fprintf(stderr, "ERROR: regex /%s/ failed due to unknown reason.\n", regex);
        exit(1);
    }

    i = 0; // i now acts as a subject index
    while (subject[i]) {
        matchLen = matchPosition(&subject[i], &regex[begin], regexLen-begin);

        if (mustMatchEnd) {
            if ((i+matchLen) == subjectLen) {
                *point = i;
                *len   = matchLen;
                return 1;
            }
        }
        else {
            if (matchLen >= 0) {
                *point = i;
                *len   = matchLen;
                return 1;
            }
        }

        if (begin == 1) break;
        i++;
    }

    return 0;
}

/* matchPostion
 * Pre: A String subject, A String regular expression, and
 *      the length of that regular expression
 * Post: Whatever matchSubset returns.
 */
int matchPosition(char *subject, char *regex, int regexLen) {
    return matchSubset(subject, regex, regexLen);
}

/* matchSubset
 * Pre: A String subject, A String regular expression, and
 *      the length of that regular expression
 * Post: Returns the length of a successful match, or a code
 *       of NO_MATCH on failure.
 * This routine will match a regular expression against a string.
 * If it encounters a '( )' grouping, it will call matchAtom, which
 * will call matchSubset again to handle that grouping recursively.
 */
int matchSubset(char *subject, char *regex, int len) {

    int min, max;
    int regexCursor = 0;
    int parens = 0;
    int atomLength;
    int atomMatchLength = NO_MATCH;
    int atomAlternate = NO_MATCH;
    int atomMatchLengthRest;
    int loops = INFINITY;
    int greed = GREEDY;

    // If we hit a pipe, stop. We don't care about pipes this early.
    if (regex[regexCursor] == '|' || regex[regexCursor] == ')' || len == 0)
        return 0;

    // Get First Atom
    if      (regex[regexCursor] == '[') {
        while (regexCursor < len) {
            if (regex[regexCursor] == ']') { regexCursor++; break; }
            if (regex[regexCursor] == '\\') regexCursor++;
            regexCursor++;
        }
    }
    else if (regex[regexCursor] == '(' && regexCursor < len) {
        int insideClass = 0;
        parens = 0;
        regexCursor++;
        while (regexCursor < len) {
            if (regex[regexCursor] == '[') insideClass = 1;
            if (regex[regexCursor] == ']') insideClass = 0;

            if (insideClass == 0) {
                if (regex[regexCursor] == ')' && parens == 0) { regexCursor++; break; }
                if (regex[regexCursor] == ')') parens--;
                if (regex[regexCursor] == '(') parens++;
            }

            if (regex[regexCursor] == '\\') regexCursor++;
            regexCursor++;
        }
    }
    else if (regex[regexCursor] == '\\') {
        regexCursor += 2;
    }
    else if (regex[regexCursor] == '+' ||
             regex[regexCursor] == '*' ||
             regex[regexCursor] == '?' ||
             regex[regexCursor] == '{') {
        fprintf(stderr, "Error: Quantifier '%c' follows nothing in expredssion.\n", regex[regexCursor]);
        exit(1);
    }
    else {
        regexCursor++;
    }

    if (regexCursor > len) {
        fprintf(stderr, "Expression failed.\n");
        exit(1);
    }

    atomLength = regexCursor;

    // Get Modifier
    if      (regex[regexCursor] == '*') {
        regexCursor++;
        min = 0;
        max = INFINITY;
    }
    else if (regex[regexCursor] == '+') {
        regexCursor++;
        min = 1;
        max = INFINITY;
    }
    else if (regex[regexCursor] == '?') {
        regexCursor++;
        min = 0;
        max = 1;
    }
    else if (regex[regexCursor] == '{') {
        regexCursor++;
        min = 0;
        max = INFINITY;

        if ('0' <= regex[regexCursor] && regex[regexCursor] <= '9')
            sscanf(&regex[regexCursor], "%d", &min);

        while (regexCursor < len) {
            if (regex[regexCursor] == ',') { break; }
            if (regex[regexCursor] == '}') { break; }
            regexCursor++;
        }

        if (regex[regexCursor] == ',') {
            regexCursor++;
            if ('0' <= regex[regexCursor] && regex[regexCursor] <= '9')
                sscanf(&regex[regexCursor], "%d", &max);

            while (regexCursor < len) {
                if (regex[regexCursor] == '}') { regexCursor++; break; }
                regexCursor++;
            }
        }
        else if (regex[regexCursor] == '}') {
            regexCursor++;
            max = min;
        }
    }
    else {
        min = 1;
        max = 1;
    }

    if (regexCursor > len) {
        fprintf(stderr, "Expression failed.\n");
        exit(1);
    }

    // Get Lazy/Posessive Modifier
    if      (regex[regexCursor] == '+') {
        regexCursor++;
        greed = POSESSIVE;
    }
    else if (regex[regexCursor] == '?') {
        regexCursor++;
        greed = LAZY;
        loops = min;
    }

    while (1) {
        atomMatchLength = matchAtom(subject, regex, atomLength, min, max, &loops);

        // If successful
        if (atomMatchLength >= 0) {
            if (regexCursor <= len) {
                atomMatchLengthRest = matchSubset(&subject[atomMatchLength], &regex[regexCursor], len-regexCursor);
                if (atomMatchLengthRest >= 0) {
                    atomMatchLength += atomMatchLengthRest;
                    break;
                }

                atomMatchLength = NO_MATCH;
                if      (greed == GREEDY) {
                    if (loops <= min)
                        break;

                    loops--;
                }
                else if (greed == LAZY) {
                    if (max != INFINITY && loops >= max)
                        break;

                    loops++;
                }
                else if (greed == POSESSIVE) {
                    break;
                }
            }
        }
        else
            break;
    }

    if (atomMatchLength < 0) {
        // Scan to next pipe, go one character further, and recalculate.
        parens = 0;
        while (regexCursor < len) {
            if (parens == 0 && regex[regexCursor] == '|') break;
            if (parens == 0 && regex[regexCursor] == ')') break;
            if (regex[regexCursor] == '(')  parens++;
            if (regex[regexCursor] == ')')  parens--;
            if (regex[regexCursor] == '\\') regexCursor++;
            regexCursor++;
        }

        if (regexCursor > len) {
            fprintf(stderr, "Expression failed.\n");
            exit(1);
        }
    }

    if (regex[regexCursor] == '|') {
        regexCursor++;
        atomAlternate = matchSubset(subject, &regex[regexCursor], len-regexCursor);

        if (atomAlternate > atomMatchLength)
            return atomAlternate;
    }

    return atomMatchLength;
}

/* matchAtom
 * Pre: A string subject,
 *      A string regular expression
 *      The length of that regular expression
 *      A minimum count requirement that regex must match
 *      A maximum count requirement that regex must match
 *      A desired count that regex must match (which also becomes a post)
 * Post: Returns the length of a successful match (or a code of NO_MATCH),
 *       The 'desired' variable returns the number of loops attempted,
 *       regardless of success or failure.
 *
 *  If the 'desired' variable is set to INFINITY, this loop will rely on min
 *  and max to determine when to stop. If the 'desired' variable is set to
 *  anything else, it dictates when the matching stops. If the desired count
 *  is not met, the method returns a failure, even if the min and max could
 *  be successful.
 *
 */
int matchAtom(char *subject, char *regex, int len, int min, int max, int *desired) {

    int matchLength;
    int t = 0; // Total Match Length
    int i = 0; // Total Matches

    while (1) {
        if (i == *desired)
            break;

        if (max == 0)
            break;

        matchLength = NO_MATCH;
        if ('\0' == subject[t])
            break;

        // Attempt Match
        if      (regex[0] == '(') {
            matchLength = matchSubset(&subject[t], &regex[1], len-1);
        }
        else if (regex[0] == '[') {
            if (characterClass(regex, subject[t])) matchLength = 1;
        }
        else if (regex[0] == '\\') {
            char p = getEscapeChar(regex[1]);

            switch (p) {
                case 'w': if (characterClass("[a-zA-Z0-9_]", subject[t]))  matchLength = 1;
                          break;
                case 'W': if (characterClass("[^a-zA-Z0-9_]", subject[t])) matchLength = 1;
                          break;
                case 'd': if (characterClass("[0-9]", subject[t]))         matchLength = 1;
                          break;
                case 'D': if (characterClass("[^0-9]", subject[t]))        matchLength = 1;
                          break;
                case 's': if (characterClass("[ \t\n]", subject[t]))       matchLength = 1;
                          break;
                case 'S': if (characterClass("[^ \t\n]", subject[t]))      matchLength = 1;
                          break;
                default:  if (p == subject[t])                             matchLength = 1;
            }
        }
        else {
            // The '.' expression matches anything.
            if (regex[0] == subject[t] || regex[0] == '.') matchLength = 1;
        }

        // If Not Successful...
        if (matchLength < 0)
            break;

        t += matchLength;
        i++;
        min--;
        max--;
    }

    // If desired matches has not been reached, then fail.
    if (*desired != -1 && *desired != i)
        return NO_MATCH;

    // If Failure
    if (min > 0)
        return NO_MATCH;

    *desired = i;
    return t;
}

#endif
