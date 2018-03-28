# -*- coding: utf-8 -*-

import system_tests


class TestCvePoC(system_tests.Case):

    url = "https://github.com/Exiv2/exiv2/issues/51"

    filename = "{data_path}/POC4"
    commands = ["{exiv2} " + filename]
    stdout = [""]
    stderr = ["""{exiv2_exception_message} """ + filename + """:
{kerInvalidMalloc}
"""]
    retval = [1]
