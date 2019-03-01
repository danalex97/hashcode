#include <unordered_map>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <set>
#include <iterator>
#include <unordered_map>
#include <thread>
using namespace std;

int n;
struct photo_t {
	char t;
	int x, y;
	vector<int> tags;

	photo_t(char t, int x) {
		this->t = t;
		this->x = x;
		this->y = -1;
		tags = vector<int>();
	}

	int dist(photo_t p) {
		vector<int> v;
		set_intersection(
			tags.begin(), tags.end(),
            p.tags.begin(), p.tags.end(),
            back_inserter(v)
		);

		int i = v.size();
		int j = tags.size() - i;
		int k = p.tags.size() - i;

		return min(min(i, j), k);
	}
};

int dist(photo_t x, photo_t y) {
	return x.dist(y);
}

vector<photo_t> to_horizontal(vector<photo_t> v) {
	vector<photo_t> res;
	for (int i = 0; i < v.size(); i += 2) {
		photo_t p('H', v[i].x);

		set<int> tags;
		// put first set of tags
		for (auto &tag : v[i].tags) {
			tags.insert(tag);
		}
		if (i + 1 < v.size()) {
			// put second set of tags
			for (auto &tag : v[i + 1].tags) {
				tags.insert(tag);
			}
			p.y = v[i + 1].x;
		}

		for (auto &t : tags) {
			p.tags.push_back(t);
		}

		res.push_back(p);
	}
	return res;
}


vector<photo_t> reorder(vector<photo_t> nodes) {
	int n = nodes.size();
	vector< vector<int> > dst(n, vector<int>(n));

	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n; ++j) {
			if (i != j) {
				dst[i][j] = nodes[i].dist(nodes[j]);
			}
		}
	}

	// dp[c][x]  = min dist to c, x
	// dad[c][x] = coming from configuration...

	vector< vector<int> > dp(1 << n, vector<int>(n, -1));
	vector< vector<int> > dad(1 << n, vector<int>(n, -2));

	for (int i = 0; i < n; ++i) {
		dp[1 << i][i] = 0;
		dad[1 << i][i] = -1;
	}

	for (int c = 1; c <= 1 << n; ++c) {
		for (int x = 0; x < n; ++x) {
			if ((1 << x) & c) {
				for (int y = 0; y < n; ++y) {
					int c2 = (1 << y) & c;
					if (c2 && x != y && dp[c2][y] != -1) {
						if (dp[c][x] < dp[c ^ (1 << x)][y] + dst[x][y]) {
							dp[c][x] = dp[c ^ (1 << x)][y] + dst[x][y];
							dad[c][x] = y;
						}
					}
				}
			}
		}
	}

	int lst = 0;
	int mx = -1;
	for (int x = 0; x < n; ++x) {
		if (dp[(1 << n) - 1][x] > mx) {
			mx = dp[(1 << n) - 1][x];
			lst = x;
		}
	}

	vector<int> idx;
	int c = (1 << n) - 1;
	int x = lst;
	while (x != -1) {
		idx.push_back(x);

		int y = dad[c][x];
		c -= 1 << x;
		x = y;
	}

	vector<photo_t> ans;
	for (int i = n - 1; i >= 0; --i) {
		ans.push_back(nodes[idx[i]]);
	}

	return ans;
}


bool operator < (const photo_t& a, const photo_t& b) {
  return a.x < b.x;
}

bool operator == (const photo_t& a, const photo_t& b) {
	return a.x == b.x;
}

vector< vector<photo_t> > outs;
vector< set<photo_t> > st;

void group_close(int idx) {
	photo_t curr = *st[idx].begin();
	st[idx].erase(st[idx].begin());
	outs[idx].push_back(curr);

	int count = 0;
	while (!st[idx].empty()) {
		count++;
		photo_t best = curr;

		// take best photo
		int best_score = -1;
		for (auto &photo : st[idx]) {
			int score = dist(photo, curr);
			if (score > best_score || (score == best_score && photo.tags.size() < best.tags.size())) {
				best_score = score;
				best = photo;
			}
		}

      	st[idx].erase(best);
    	outs[idx].push_back(best);
    	curr = best;
  	}
}

vector< vector<photo_t> > make_groups(vector<photo_t> h, int group_size) {
	vector< vector<photo_t> > groups;
	vector<photo_t> group;
	for (auto &x : h) {
		group.push_back(x);
		if (group.size() == group_size) {
			groups.push_back(group);
			group = vector<photo_t>();
		}
	}
	if (group.size() > 0) {
		groups.push_back(group);
	}
	return groups;
}

int main() {
	cin >> n;
	vector<photo_t> v;
	vector<photo_t> h;
  	unordered_map<string, int> mp;
    int str_counter = 0;
	for (int i = 0; i < n; ++i) {
		int m;
		char t;

		cin >> t >> m;
		photo_t pp(t, i);

		for (int j = 0; j < m; ++j) {
			string str;
			cin >> str;

			if (!mp[str]) {
				mp[str] = ++str_counter;
			}
			pp.tags.push_back(mp[str]);
		}

		sort(pp.tags.begin(), pp.tags.end());
		if (pp.t == 'H') {
			h.push_back(pp);
		} else {
			v.push_back(pp);
		}
	}

	// shuffle inputs
	random_shuffle(v.begin(), v.end());
	random_shuffle(h.begin(), h.end());

	// solve vertical
	for (auto &x : to_horizontal(v)) {
		h.push_back(x);
	}

	// group sparse
	auto groups = make_groups(h, 20000);
	vector< thread > thds;
	outs = vector< vector<photo_t> >(groups.size());
	st = vector< set<photo_t> >(groups.size());
	int ctr = 0;
	for (auto &group : groups) {
		st[ctr] = set<photo_t>(group.begin(), group.end());
		thds.push_back(thread(group_close, ctr));
		ctr++;
	}

	for (auto &t : thds) {
		t.join();
		cerr << "> group finished\n";
	}

	vector<photo_t> phase1;
	for (auto &g : outs) {
		for (auto &x : g) {
			phase1.push_back(x);
		}
	}

	cerr << "> phase 2 started\n";
	// group local
	vector<photo_t> ans;
	for (auto &group : make_groups(phase1, 15)) {
		for (auto &x : reorder(group)) {
			ans.push_back(x);
		}
	}

	cout << ans.size() << '\n';
	for (auto &x : ans) {
		cout << x.x;
		if (x.y != -1) {
			cout << ' ' << x.y;
		}
		cout << '\n';
	}
}
