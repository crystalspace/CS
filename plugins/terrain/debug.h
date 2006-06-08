#pragma once

#define PR_TOSTR_HELPER(line) #line
#define PR_TOSTR(line) PR_TOSTR_HELPER(line)
#define PR_WARNING(message) __FILE__"("PR_TOSTR(__LINE__)"): warning: " ## message
