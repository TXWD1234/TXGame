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
		      faceUp(size) {}

		std::vector<float> begin;
		std::vector<float> end;
		std::vector<float> offset;
		std::vector<tx::u8> faceUp; // use u8 as boolean

		size_t size() const { return begin.size(); };
	};



public:
	struct Initer {
		friend class Platformer;

		void addHorizontal(float xBegin, float xEnd, float y, bool faceUp) {
			horizontal.begin.push_back(xBegin);
			horizontal.end.push_back(xEnd);
			horizontal.offset.push_back(y);
			horizontal.faceUp.push_back(faceUp);
		}

	private:
		Constraints_impl vertical;
		Constraints_impl horizontal;
	};

	template <std::invocable<Initer&> Func>
	Platformer(Func&& initFunc) {
		Initer initer;
		initFunc(initer);
		init_impl(std::move(initer));
	}







private:
	Constraints_impl vertical;
	Constraints_impl horizontal;

	void init_impl(Initer&& initer) {
		vertical = std::move(initer.vertical);
		horizontal = std::move(initer.horizontal);
	}
	Constraints_impl sortConstaints_impl(Constraints_impl& c) {
		std::vector<tx::u32> indexArray(c.size());
		for (tx::u32 i = 0; i < c.size(); i++) {
			indexArray[i] = i;
		}

		std::sort(indexArray.begin(), indexArray.end(), [&](tx::u32 a, tx::u32 b) -> bool {
			return c.begin[a] < c.begin[b];
		});

		Constraints_impl sorted(c.size());
		for (tx::u32 i = 0; i < c.size(); i++) {
			sorted.begin[i] = c.begin[indexArray[i]];
			sorted.end[i] = c.end[indexArray[i]];
			sorted.offset[i] = c.offset[indexArray[i]];
			sorted.faceUp[i] = c.faceUp[indexArray[i]];
		}

		return sorted;
	}
};