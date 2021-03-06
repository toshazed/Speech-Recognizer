#ifndef MFCCENTRY_H_
#define MFCCENTRY_H_

#include <fstream>

using namespace std;

namespace yazz {
namespace model {

class MfccEntry {

public:
	MfccEntry();
	MfccEntry(double* data);
	~MfccEntry();

	inline size_t getSize() const { return this->size; }
	inline double* getData() const { return this->data; }

	friend std::ostream& operator<<(std::ostream& fs, const MfccEntry& obj);
	friend std::istream& operator>>(std::istream& fs, MfccEntry& obj);

	void print();
private:
	size_t	size;
	double*	data;
};

} /* namespace model */
} /* namespace yazz */

#endif /* MFCCENTRY_H_ */
