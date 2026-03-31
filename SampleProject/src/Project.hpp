// #define TX_JERRY_IMPL
// #include "TXLib/txlib.hpp"
// #include "TXLib/txgraphics.hpp"
// #include "TXLib/txutility.hpp"
// #include "TXLib/txmath.hpp"
// #include "TXLib/txmap.hpp"
// #include "TXLib/txjson.hpp"

#include "utility_dump.hpp"



class Platformer {
private:
	struct Constraints_impl {
		Constraints_impl(tx::u32 size)
		    : begin(size),
		      end(size),
		      offset(size),
		      facePositive(size) {}
		Constraints_impl() = default;

		std::vector<float> begin;
		std::vector<float> end;
		std::vector<float> offset;
		std::vector<tx::u8> facePositive; // use u8 as boolean

		size_t size() const { return begin.size(); };
	};



public:
	struct Initer {
		friend class Platformer;

		void addHorizontal(float xBegin, float xEnd, float y, bool faceUp) {
			if (xBegin > xEnd) std::swap(xBegin, xEnd);
			horizontal.begin.push_back(xBegin);
			horizontal.end.push_back(xEnd);
			horizontal.offset.push_back(y);
			horizontal.facePositive.push_back(faceUp);
		}
		void addVertical(float yBegin, float yEnd, float x, bool faceRight) {
			if (yBegin > yEnd) std::swap(yBegin, yEnd);
			vertical.begin.push_back(yBegin);
			vertical.end.push_back(yEnd);
			vertical.offset.push_back(x);
			vertical.facePositive.push_back(faceRight);
		}

	private:
		Constraints_impl vertical;
		Constraints_impl horizontal;
	};

	Platformer() = default;

	template <std::invocable<Initer&> Func>
	Platformer(Func&& initFunc) {
		Initer initer;
		initFunc(initer);
		init_impl(std::move(initer));
	}

	void moveConstrainted(tx::Rect& object, tx::vec2 movement) {
		tx::Rect target = object.offset(movement);
		float left = std::min(object.left(), target.left()),
		      right = std::max(object.right(), target.right()),
		      top = std::max(object.top(), target.top()),
		      bottom = std::min(object.bottom(), target.bottom());
		float resultX = target.bottomLeft().x(), resultY = target.bottomLeft().y();
		if (std::fabs(movement.y()) > tx::epsilon)
			solveConstraint(horizontal, resultY, target.width(),
			                movement.y() > 0, left, right, bottom, top);
		if (std::fabs(movement.x()) > tx::epsilon)
			solveConstraint(vertical, resultX, target.height(),
			                movement.x() > 0, bottom, top, left, right);
		object.setPos(resultX, resultY);
	}







private:
	Constraints_impl vertical;
	Constraints_impl horizontal;

	void init_impl(Initer&& initer) {
		vertical = std::move(initer.vertical);
		horizontal = std::move(initer.horizontal);
		sortConstaints_impl(vertical);
		sortConstaints_impl(horizontal);
	}
	void sortConstaints_impl(Constraints_impl& c) {
		tx::multi_sort(c.end.begin(), c.end.end(), c.begin.begin(), c.facePositive.begin(), c.offset.begin());
	}

	// parameter begin should be the negative end of the axis (eg. left / bottom)
	tx::u32 findBeginIndex_impl(const Constraints_impl& cons, float begin) const {
		return std::lower_bound(cons.end.begin(), cons.end.end(), begin) - cons.end.begin();
	}
	void solveConstraint(const Constraints_impl& cons, float& result, float objSize,
	                     bool facePositive, float negativeEdge, float positiveEdge, float oppositeNegativeEdge, float oppositePositiveEdge) {
		tx::u32 indexBegin = findBeginIndex_impl(cons, negativeEdge);
		if (indexBegin == cons.size()) return;
		for (tx::u32 i = indexBegin; i < cons.size(); i++) {
			if (cons.begin[i] > positiveEdge) break;
			if (cons.facePositive[i] == facePositive ||
			    cons.offset[i] >= oppositePositiveEdge ||
			    cons.offset[i] <= oppositeNegativeEdge) continue;
			if (facePositive)
				result = std::min(result, cons.offset[i] - objSize);
			else
				result = std::max(result, cons.offset[i]);
		}
	}
};