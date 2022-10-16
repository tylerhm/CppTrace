#ifndef PROGRESS_WRITER_H
#define PROGRESS_WRITER_H

#include <iostream>

using std::cout;

struct ProgressIndicator {
  const int BAR_WIDTH = 70;

  int numSteps;
  explicit ProgressIndicator(const int numSteps)
      : numSteps(numSteps) { indicate(0); }

  void indicate(const int curStep) const {
    double progress = (double) curStep / numSteps;
    int intProgress = (int) (progress * 100);

    int barProgress = (int) (BAR_WIDTH * progress);
    cout << "[";
    for (int i = 0; i < BAR_WIDTH; i++) {
      if (i < barProgress)
        cout << "=";
      else if (i == barProgress)
        cout << ">";
      else
        cout << "-";
    }
    cout << "] ";
    cout << intProgress << "%\r";
    cout.flush();
  }

  void done() const {
    indicate(numSteps);
    cout << "\n";
    cout.flush();
  }
};

#endif