/*
 * TestClass.h
 *
 *  Created on: Feb 20, 2014
 *      Author: root
 */

#ifndef TESTCLASS_H_
#define TESTCLASS_H_

namespace na62 {

class TestClass {
public:
	TestClass();
	virtual ~TestClass();
	bool isValid(){
		return true;
	}
};

} /* namespace na62 */

#endif /* TESTCLASS_H_ */
