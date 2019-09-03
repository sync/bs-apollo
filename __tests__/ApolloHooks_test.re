open Jest;
open Expect;

%raw
"global.fetch.mockRejectOnce(new Error('something is bad'))";

module SubredditQuery = [%graphql
  {|
query GetSubreddit($name: String!) {
    subreddit(name: $name) {
      posts {
        id
        title
      }
    }
  }
|}
];

let query = SubredditQuery.make(~name="reactjs", ());

module ApolloClient = {
  type t;

  [@bs.deriving abstract]
  type config = {uri: string};

  [@bs.module "apollo-boost"] [@bs.new] external make: config => t = "default";
};

module ApolloProvider = {
  [@bs.module "@apollo/react-hooks"] [@react.component]
  external make:
    (~client: ApolloClient.t, ~children: React.element) => React.element =
    "ApolloProvider";
};

module TestComponent = {
  [@react.component]
  let make = () => {
    let result = ApolloHooks.useQuery(~query);

    switch (result) {
    | ApolloHooks.Loading => <div> "Loading"->React.string </div>
    | ApolloHooks.Error(message) => <div> message->React.string </div>
    | ApolloHooks.Data(response) =>
      switch (response##subreddit) {
      | Some(subreddit) =>
        <ul>
          {subreddit##posts
           |> Array.map(post =>
                <li key={post##id}> {post##title->React.string} </li>
              )
           |> ReasonReact.array}
        </ul>
      | _ => <div> "No stories found"->React.string </div>
      }
    };
  };
};

describe("useQuery", () =>
  testPromise("returns an error", () => {
    open ReactTestingLibrary;

    let client =
      ApolloClient.make(ApolloClient.config(~uri="localhost:666"));
    let result =
      ReactTestingLibrary.render(
        <ApolloProvider client> <TestComponent /> </ApolloProvider>,
      );

    DomTestingLibrary.waitForElement(
      ~callback=
        () =>
          result
          |> ReactTestingLibrary.getByText(
               ~matcher=`Str("Network error: something is bad"),
             ),
      (),
    )
    |> Js.Promise.then_(_ =>
         result
         |> getByText(~matcher=`Str("Network error: something is bad"))
         |> expect
         |> ExpectJs.toBeTruthy
         |> Js.Promise.resolve
       );
  })
);
