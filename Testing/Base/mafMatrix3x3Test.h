/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: mafMatrix3x3Test.h,v $
Language:  C++
Date:      $Date: 2006-11-06 15:30:02 $
Version:   $Revision: 1.1 $
Authors:   Paolo Quadrani
==========================================================================
Copyright (c) 2002/2004 
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

#ifndef __CPP_UNIT_mafMatrix3x3Test_H__
#define __CPP_UNIT_mafMatrix3x3Test_H__

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>

/** Test for mafMatrix; Use this suite to trace memory problems */
class mafMatrix3x3Test : public CPPUNIT_NS::TestFixture
{
public: 
  // CPPUNIT fixture: executed before each test
  void setUp();

  // CPPUNIT fixture: executed after each test
  void tearDown();

  // CPPUNIT test suite
  CPPUNIT_TEST_SUITE( mafMatrix3x3Test );
  CPPUNIT_TEST(TestFixture); // just to test that the fixture has no leaks
  CPPUNIT_TEST(TestStaticAllocation);
  CPPUNIT_TEST(TestDynamicAllocation);
  CPPUNIT_TEST(TestCopy);
  CPPUNIT_TEST(TestZero);
  CPPUNIT_TEST(TestGetVersor);
  CPPUNIT_TEST(TestIdentity);
  CPPUNIT_TEST(TestMultiplyVector);
  CPPUNIT_TEST(TestMultiply);
  CPPUNIT_TEST(TestTranspose);
  CPPUNIT_TEST(TestInvert);
  CPPUNIT_TEST(TestDeterminant);
  CPPUNIT_TEST_SUITE_END();

private:
  void TestFixture();
  void TestStaticAllocation();
  void TestDynamicAllocation();
  void TestCopy();
  void TestZero();
  void TestGetVersor();
  void TestIdentity();
  void TestMultiplyVector();
  void TestMultiply();
  void TestTranspose();
  void TestInvert();
  void TestDeterminant();

  bool result;
};

int
main( int argc, char* argv[] )
{
  // Create the event manager and test controller
  CPPUNIT_NS::TestResult controller;

  // Add a listener that collects test result
  CPPUNIT_NS::TestResultCollector result;
  controller.addListener( &result );        

  // Add a listener that print dots as test run.
  CPPUNIT_NS::BriefTestProgressListener progress;
  controller.addListener( &progress );      

  // Add the top suite to the test runner
  CPPUNIT_NS::TestRunner runner;
  runner.addTest( mafMatrix3x3Test::suite());
  runner.run( controller );

  // Print test in a compiler compatible format.
  CPPUNIT_NS::CompilerOutputter outputter( &result, CPPUNIT_NS::stdCOut() );
  outputter.write(); 

  return result.wasSuccessful() ? 0 : 1;
}
#endif
