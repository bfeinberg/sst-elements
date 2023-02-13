# -*- coding: utf-8 -*-
from datetime import datetime
import os
import subprocess

from sst_unittest import *
from sst_unittest_support import *

################################################################################

class testcase_hg(SSTTestCase):

    @classmethod
    def setUpClass(cls):
        hg_dir = subprocess.run(["sst-config", "SST_ELEMENT_TESTS", "mercury"],
                                capture_output=True)
        strip = hg_dir.stdout.rstrip()
        os.chdir(strip)
        subprocess.run("./build.sh")


    def setUp(self):
        super(testcase_hg, self).setUp()
        # Put test based setup code here. it is called once before every test

    def tearDown(self):
        # Put test based teardown code here. it is called once after every test
        super(testcase_hg, self).tearDown()

#####

    def test_testme(self):
        # get timestamp 1
        ts1 = datetime.now()
        # now = date.fromtimestamp(time.time())
        # run command
        self.simple_components_template("testme")
        # get timestamp 2
        # make sure ts2 - ts1 < 5 (sleep time from testme.cc)
        pass

    def test_node(self):
        self.simple_components_template("example1")

    def test_basic_clocks(self):
        self.simple_components_template("basicClocks")

    def test_basic_links(self):
        self.simple_components_template("basicLinks")

    def test_basic_params(self):
        self.simple_components_template("basicParams")

    def test_basic_statistics_0(self):
        self.simple_components_template("basicStatistics0")

    def test_basic_statistics_1(self):
        self.simple_components_template("basicStatistics1")

    def test_basic_statistics_2(self):
        self.simple_components_template("basicStatistics2")

    #def test_simple_rng_component_marsaglia(self):
    #    self.simple_components_template("simpleRNGComponent_marsaglia", striptotail=1)

#####

    def simple_components_template(self, testcase, striptotail=0):
        # Get the path to the test files
        test_path = self.get_testsuite_dir()
        outdir = self.get_test_output_run_dir()
        tmpdir = self.get_test_output_tmp_dir()

        # Set the various file paths
        testDataFileName="{0}".format(testcase)

        sdlfile = "{0}/{1}.py".format(test_path, testDataFileName)
        reffile = "{0}/refFiles/{1}.out".format(test_path, testDataFileName)
        outfile = "{0}/{1}.out".format(outdir, testDataFileName)
        tmpfile = "{0}/{1}.tmp".format(tmpdir, testDataFileName)
        cmpfile = "{0}/{1}.cmp".format(tmpdir, testDataFileName)
        errfile = "{0}/{1}.err".format(outdir, testDataFileName)
        mpioutfiles = "{0}/{1}.testfile".format(outdir, testDataFileName)

        self.run_sst(sdlfile, outfile, errfile, mpi_out_files=mpioutfiles)

        testing_remove_component_warning_from_file(outfile)

        # Copy the outfile to the cmpfile
        os.system("cp {0} {1}".format(outfile, cmpfile))

        if striptotail == 1:
            # Post processing of the output data to scrub it into a format to compare
            os.system("grep Random {0} > {1}".format(outfile, tmpfile))
            os.system("tail -5 {0} > {1}".format(tmpfile, cmpfile))

        # NOTE: THE PASS / FAIL EVALUATIONS ARE PORTED FROM THE SQE BAMBOO
        #       BASED testSuite_XXX.sh THESE SHOULD BE RE-EVALUATED BY THE
        #       DEVELOPER AGAINST THE LATEST VERSION OF SST TO SEE IF THE
        #       TESTS & RESULT FILES ARE STILL VALID

        # Perform the tests
        if os_test_file(errfile, "-s"):
            log_testing_note("hg test {0} has a Non-Empty Error File {1}".format(testDataFileName, errfile))

        cmp_result = testing_compare_sorted_diff(testcase, cmpfile, reffile)
        if (cmp_result == False):
            diffdata = testing_get_diff_data(testcase)
            log_failure(diffdata)
        self.assertTrue(cmp_result, "Sorted Output file {0} does not match sorted Reference File {1}".format(cmpfile, reffile))
