type queryString;

type gql = (. string) => queryString;

[@bs.module "graphql-tag"] external gql: gql = "default";

type apolloError = {message: string};

type clientRequestResult('data) = {
  loading: bool,
  error: option(apolloError),
  data: 'data,
};

type queryResponse('a) =
  | Loading
  | Error(string)
  | Data('a);

[@bs.module "@apollo/react-hooks"]
external _useQuery:
  (queryString, Js.Dict.t('a)) => clientRequestResult('data) =
  "useQuery";

let useQuery = (~query) => {
  let graphqlQueryAST = gql(. query##query);
  let options = Js.Dict.fromList([("variables", query##variables)]);
  let result = _useQuery(graphqlQueryAST, options);

  switch (result.loading, result.error, result.data) {
  | (true, _, _) => Loading
  | (false, _, Some(response)) => Data(response |> query##parse)
  | (false, Some(error), None) => Error(error.message)
  | _ => Error("Something is wrong")
  };
};
