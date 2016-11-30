/*
 *   Copyright 2012, valabau
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 * 
 * Range.h
 *
 *  Created on: 13/09/2012
 *      Author: valabau
 */

#ifndef CASMACAT_RANGE_H_
#define CASMACAT_RANGE_H_

namespace casmacat {

/**
 * Structures for manipulating ranges
 */

  class Range {
    size_t from;
    size_t to;
  public:
    Range(): from(0), to(0) {}
    Range(size_t _start, size_t _end): from(_start), to(_end) {}
    std::ostream& print(std::ostream& out) const {
      return out << "(" << from << "," << to << ")";
    }
  };

  std::ostream& operator<<(std::ostream& out, const Range &range) {
    return range.print(out);
  }

  class RangeSequence {
    std::vector<Range> sequence;
  public:
    RangeSequence() {}
    void addRange(const Range &range)  { sequence.push_back(range); }
    void add(size_t start, size_t end) { sequence.push_back(Range(start, end)); }
    std::ostream& print(std::ostream& out) const {
      out << "[";
      copy(sequence.begin(), sequence.end(), std::ostream_iterator<Range>(out, ";"));
      out << "]";
      return out;
    }
  };

  std::ostream& operator<<(std::ostream& out, const RangeSequence &range_requence) {
    return range_requence.print(out);
  }
}


#endif /* CASMACAT_RANGE_H_ */
