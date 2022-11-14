# Knight's Tour - An improvised Warnsdorff's algorithm
*****
This was done as part of an assignment of course CS F372 - Operating Systems at BITS Pilani.
*****
## Problem Statement : 

To solve the knight’s tour problem given an NxN board and the starting position (X, Y) of the knight. <br>
**Constraints** : <br>
`0 <= N <= 50` <br>
`0 <= X, Y <= N - 1` (Zero-indexed)<br>

## Building an algorithm : 

### Backtracking : 

- The first and foremost approach was an ***O(8^N^2)*** backtracking approach, which would see all the possibilites of a knight and then check if the target square can be reached from the given start position.
- But this approach is very heavy on computation, and hence comes with a very high time complexity. The obvious step forward was to try and reduce the time taken.

### Forking : 

- The next approach was to fork at every possible move of the knight, but this would result in a lot of copies of the process being created, and hence causing a Memory overhead, which is undesirable. Hence, this method was also not the best one.

###  Multithreading : 

- Creating threads to discover every possibility would mean that we are creating a lot of threads, and since creating threads also comes with an overhead, it would _backfire_ to our algoirthm.
- Another method could be to just randomly walk through all possibilites and _hope_ you find the result soon enough. But this was something that was very inconsistent. We need something better.

### Warnsdorff's algorithm I: 

- In brevity, this algorithm makes sure that the knight is moved such that it always proceeds to a square from which the knight will have fewest onward moves, i.e, we choose to go to the minimum degree square. [*Degree refers to how many viable options are there for the knight to move from a given square* ]
- Here, we do not use any multi-threading, just a simple warnsdorff. But this also seemed to fail a few test cases, especially as the board size became close to the constraints. Infact, it also failed for a few cases of 8x8 chessboard.
- This algorithms works in a time complexity of ***O(N^2)***

### Warnsdorff's algorithm II :

- Now, instead of choosing a square from using Warnsdorff initially itself, we create 8 threads(*there is no specific reason as to why ONLY 8 threads, but it was more of an educated guess - mostly because most laptops and PCs use 8 cores, and because there are 8 possibilites for a knight to go to*)
- Following this, we use the same Warnsdorff's I (aforementioned) which carries on from each of the 8 newly reached positions from the threads. Keep in mind everything is happening at the same time, since they are threads.
- The problem here lies in cases where there might be a tie between different squares(i.e same minimum degree for more than 1 square). Mind that Warnsdorff randomly chooses one of the viable minimum degree nodes and searches for the path. But this might not ALWAYS result in a right solution.

### Warnsdorff's algorithm with tie-breaking : 

- It was evident from the previous approach that there has to be a better heuristic to be followed to tie-break. After having researched a bit, the solution was to implement one of the following :
  - ***Ira Paul's method*** : On encountering conflicting squares, we take sum of degrees of all univisted neighbours of the tied nodes and choose to jump   to the square with the minimum sum. 
  - ***Arnd Roth's method*** : This heuristic proposes that we go to the node with maximum distance from center(*which intuitively makes sense, because as   close we go to the edges of the board, the lesser no. of viable options it can have and hence we are covering the more unreachable node first*)

- The obvious doubt that arises here is " What if the ties are for more than one time, i.e., in multiple levels ?" The answer to this lies in the fact that the occurrence of this is quite rare(first happens at 428 rows grid according to Arnd Roth's method, and has an occurrence rate of less than 1% till 2000 rows). Having followed the first method, if we applied the same methodology in every level, it would basically mean that we are increasing our overhead and might actually be as worse as an exhaustive search then, hence defeating our purpose.

### Final improvised Warnsdorff's algorithm, specific to the given constraints : 

- We finally decided to go ahead with the second method, that is to calculate the distance from center for tie-breaking.
- But for the sake of completeness, which might not be a very plausible condition to happen with the given constraints, if it so happens that the square with maximum distance from center was not the right choice, we do the following : 
  - We go ahead and if we do not find an answer in the first choice, then we back-track and choose the next maximum distance from center to search for the answer, and go on to do so on and so forth.
  - But one thing we take care of here to not adversely affect our time consumed is that we kill a thread if it runs for more than a stipulated amount of time, and ***ASSUME*** that the tour does not exist, which is not a very far-off guess because Warnsdorff is experimentally seen to be very efficient in most cases, especially with the given constraints. The stipulated time we choose to go forward with is 1 second, which shall be a safe limit considering that even big boards with 2000x20000 dimensions have been seen to complete in a time of 2-3 seconds.

### Conclusion : 

Does the above algorithm ***GUARANTEE*** an answer? The honest answer is NO. And there is no way to prove or disprove it. But it is just a **"HEURISTIC"** that is believed to be very successful, and this improvised version makes it even more probable to find the right answers in a shorter time, keeping in mind the given constraints.

