#include <iostream>
#include <set>
#include <algorithm>
#include <vector>
#include <unordered_map>
using namespace std;

int n;
struct photo_t {
	char t;
	int x, y;
	vector<string> tags;

	photo_t(char t, int x) {
		this->t = t;
		this->x = x;
		this->y = -1;
		tags = vector<string>();
	}

	int dist(photo_t p) {
		vector<string> v;
		set_intersection(
			tags.begin(), tags.end(),
            p.tags.begin(), p.tags.end(),
            back_inserter( v )
		);

		int i = v.size();
		int j = tags.size() - i;
		int k = p.tags.size() - i;

		return min(min(i, j), k);
	}
};

vector<photo_t> to_horizontal(vector<photo_t> v) {
	vector<photo_t> res;
	for (int i = 0; i < v.size(); i += 2) {
		photo_t p('H', v[i].x);

		set<string> tags;
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


int main() {
	cin >> n;
	vector<photo_t> v;
	vector<photo_t> h;
	for (int i = 0; i < n; ++i) {
		int m;
		char t;

		cin >> t >> m;
		photo_t pp(t, i);

		for (int j = 0; j < m; ++j) {
			string str;
			cin >> str;
			// cout << str << '\n';
			pp.tags.push_back(str);
		}

		sort(pp.tags.begin(), pp.tags.end());
		if (pp.t == 'H') {
			h.push_back(pp);
		} else {
			v.push_back(pp);
		}
	}

	for (auto &p : to_horizontal(v)) {
		h.push_back(p);
	// 	// for (auto &x: p.tags) {
	// 	// 	cerr << x << ' ';
	// 	// }
	// 	// cerr << '\n';
	}


	// works
	int group_size = 15;
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

	vector<photo_t> ans;
	for (auto &group : groups) {
		group = reorder(group);
		for (auto &x : group) {
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
