import os
from .settings import *
LOGGING = {}

if "TEAMCITY_VERSION" in os.environ:
    TEST_RUNNER = 'teamcity_runner.TeamCityTestRunner'