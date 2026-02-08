#ifdef RUN_TESTS

#include "../simulation.h"
#include "../tests/pch.h"
#include "../traffic_nodes.h"
#include <memory>

class IntersectionTest : public testing::Test {
protected:
	Intersection i;
	Origin o;
	Terminal r;

	static const int laneLength = 100;
	Lane in{ o, i, laneLength };
	Lane out{ i, r, laneLength };
};

TEST_F(IntersectionTest, BlocksTrafficWhenRed) {
	i.createConnection(&in, &out, Intersection::Red);

	bool actual = i.canEnter(&in);
	EXPECT_FALSE(actual);
}

TEST_F(IntersectionTest, BlocksTrafficWhenOccupied) {
	i.createConnection(&in, &out, Intersection::Green);

	std::unique_ptr<Car> car = std::make_unique<Car>();
	i.accept(&in, car.get());

	bool actual = i.canEnter(&in);
	EXPECT_FALSE(actual);
}

TEST_F(IntersectionTest, AllowsTrafficWhenGreenAndEmpty) {
	i.createConnection(&in, &out, Intersection::Green);

	bool actual = i.canEnter(&in);
	EXPECT_TRUE(actual);
}

TEST_F(IntersectionTest, ProcessAfterTickMovesCarToExitLane) {
	i.createConnection(&in, &out, Intersection::Green);

	std::unique_ptr<Car> car = std::make_unique<Car>();
	i.accept(&in, car.get());

	i.processAfterTick();

	Car* carPtr = out.findCarAt(0);
	EXPECT_EQ(car.get(), carPtr);
}

TEST_F(IntersectionTest, ProcessAfterTickHoldsCarWhenExitBlocked) {
	i.createConnection(&in, &out, Intersection::Green);

	std::unique_ptr<Car> car1 = std::make_unique<Car>();
	i.accept(&in, car1.get());

	std::unique_ptr<Car> car2 = std::make_unique<Car>();
	out.addCar(car2.get());

	i.processAfterTick();

	Car* carPtr = out.findCarAt(0);
	EXPECT_EQ(car2.get(), carPtr);
	EXPECT_FALSE(i.canEnter(&in));
}

class CarTest : public testing::Test {
protected:
	CarTest() {
		i.createConnection(&in, &out, Intersection::Red);
	}

	Intersection i;
	Origin o;
	Terminal r;

	static const int laneLength = 10;
	Lane in{ o, i, laneLength };
	Lane out{ i, r, laneLength };
};

TEST_F(CarTest, CanNotMoveWhenBlockedByCar) {
	std::unique_ptr<Car> car1 = std::make_unique<Car>();
	std::unique_ptr<Car> car2 = std::make_unique<Car>();

	in.addCar(car1.get());
	car1->setLane(&in);
	car1->accelerate();
	
	for (int j = 0; j < 5; j++) {
		car1->move();
	}

	EXPECT_EQ(10, car1->getPosition()) << "Test precondition failed: car1 is not at position 10.";

	in.addCar(car2.get());
	car2->setLane(&in);
	car2->accelerate();

	for (int j = 0; j < 4; j++) {
		car2->move();
	}

	EXPECT_EQ(8, car2->getPosition()) << "Test precondition failed: car2 is not at position 8.";

	bool result = car2->canMove();

	EXPECT_FALSE(result);
}

TEST_F(CarTest, CanNotMoveWhenBlockedByIntersection) {
	std::unique_ptr<Car> car = std::make_unique<Car>();

	in.addCar(car.get());
	car->setLane(&in);
	car->accelerate();

	for (int j = 0; j < 5; j++) {
		car->move();
	}

	EXPECT_EQ(10, car->getPosition()) << "Test precondition failed: car is not at position 10.";


	bool result = car->canMove();

	EXPECT_FALSE(result);
}

TEST_F(CarTest, CanMoveWhenNotBlockedByCar) {
	std::unique_ptr<Car> car = std::make_unique<Car>();

	in.addCar(car.get());
	car->setLane(&in);
	car->accelerate();

	for (int j = 0; j < 4; j++) {
		car->move();
	}

	EXPECT_EQ(8, car->getPosition()) << "Test precondition failed: car is not at position 8.";


	bool result = car->canMove();

	EXPECT_TRUE(result);
}

#endif