#include <iostream>
#include "SperrConfig.h"

int main()
{
  std::cout << "SPERR version: " << SPERR_VERSION_MAJOR << "." << SPERR_VERSION_MINOR << std::endl;
  std::cout << "Based on code Branch: " << SPERR_GIT_BRANCH << std::endl;
  std::cout << "Based on code SHA1  : " << SPERR_GIT_SHA1 << std::endl;
}
