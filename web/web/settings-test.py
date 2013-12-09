import os
from .settings import *
LOGGING = {}

if "teamcity.version" in os.environ:
    TEST_RUNNER = 'teamcity_runner.TeamCityTestRunner'