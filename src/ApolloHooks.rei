type queryResponse('a) =
  | Loading
  | Error(string)
  | Data('a);

let useQuery:
  (
    ~query: {
              ..
              "parse": 'a => 'b,
              "query": string,
              "variables": 'c,
            }
  ) =>
  queryResponse('b);
