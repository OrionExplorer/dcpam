--
-- PostgreSQL database cluster dump
--

SET default_transaction_read_only = off;

SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;

--
-- Roles
--

CREATE ROLE dcpam;
ALTER ROLE dcpam WITH NOSUPERUSER INHERIT NOCREATEROLE NOCREATEDB LOGIN NOREPLICATION NOBYPASSRLS PASSWORD 'md5169fc9a21551970ec3a02eb023b88c67';
CREATE ROLE postgres;
ALTER ROLE postgres WITH SUPERUSER INHERIT CREATEROLE CREATEDB LOGIN REPLICATION BYPASSRLS;






--
-- Database creation
--

CREATE DATABASE dcpam WITH TEMPLATE = template0 OWNER = postgres;
GRANT ALL ON DATABASE dcpam TO dcpam;
REVOKE CONNECT,TEMPORARY ON DATABASE template1 FROM PUBLIC;
GRANT CONNECT ON DATABASE template1 TO PUBLIC;


\connect dcpam

SET default_transaction_read_only = off;

--
-- PostgreSQL database dump
--

-- Dumped from database version 10.12 (Ubuntu 10.12-0ubuntu0.18.04.1)
-- Dumped by pg_dump version 10.12 (Ubuntu 10.12-0ubuntu0.18.04.1)

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

--
-- Name: plpgsql; Type: EXTENSION; Schema: -; Owner: 
--

CREATE EXTENSION IF NOT EXISTS plpgsql WITH SCHEMA pg_catalog;


--
-- Name: EXTENSION plpgsql; Type: COMMENT; Schema: -; Owner: 
--

COMMENT ON EXTENSION plpgsql IS 'PL/pgSQL procedural language';


SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: accounted_time; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.accounted_time (
    id integer NOT NULL,
    system_object_id integer NOT NULL,
    system_worklog_id integer NOT NULL,
    system_customer character varying(255) NOT NULL,
    system_agent_id character varying(64) NOT NULL,
    system_accounted_time numeric(10,2) NOT NULL,
    system_created timestamp without time zone NOT NULL,
    system character varying(64) NOT NULL,
    is_valid smallint NOT NULL,
    changed timestamp without time zone DEFAULT now() NOT NULL
);


ALTER TABLE public.accounted_time OWNER TO postgres;

--
-- Name: account_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.account_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.account_id_seq OWNER TO postgres;

--
-- Name: account_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.account_id_seq OWNED BY public.accounted_time.id;


--
-- Name: accounted_time id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.accounted_time ALTER COLUMN id SET DEFAULT nextval('public.account_id_seq'::regclass);


--
-- Data for Name: accounted_time; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.accounted_time (id, system_object_id, system_worklog_id, system_customer, system_agent_id, system_accounted_time, system_created, system, is_valid, changed) FROM stdin;
1	345	1274	Company Ltd.	m.kelar	35.00	2020-05-14 13:22:41.653766	OTRS	1	2020-05-19 13:50:54.434662
2	3145	13274	Company Ltd.	m.kelar	135.00	2020-05-14 13:53:39.981845	OTRS	1	2020-05-19 13:50:54.434662
3	46	5270	Company Ltd.	m.kelar	125.00	2020-05-14 13:53:40.007996	OTRS	1	2020-05-19 13:50:54.434662
4	47	5271	Company Ltd.	m.kelar	225.00	2020-05-14 13:53:40.015575	OTRS	1	2020-05-19 13:50:54.434662
5	48	5272	Company Ltd.	m.kelar	245.00	2020-05-14 13:53:40.024933	OTRS	1	2020-05-19 13:50:54.434662
6	49	5273	Company Ltd.	m.kelar	255.00	2020-05-14 13:53:40.034793	OTRS	1	2020-05-19 13:50:54.434662
7	50	5274	Company Ltd.	m.kelar	255.00	2020-05-14 13:53:40.045789	OTRS	1	2020-05-19 13:50:54.434662
8	51	5275	Company Ltd.	m.kelar	275.00	2020-05-14 13:53:40.055523	OTRS	1	2020-05-19 13:50:54.434662
9	52	5276	Company Ltd.	m.kelar	15.00	2020-05-14 13:53:40.065403	OTRS	1	2020-05-19 13:50:54.434662
10	53	5277	Company Ltd.	m.kelar	55.00	2020-05-14 13:53:40.075456	OTRS	1	2020-05-19 13:50:54.434662
11	54	5278	Company Ltd.	m.kelar	115.00	2020-05-14 13:53:40.085107	OTRS	1	2020-05-19 13:50:54.434662
12	55	5279	Company Ltd.	m.kelar	11.00	2020-05-14 13:53:40.096151	OTRS	1	2020-05-19 13:50:54.434662
13	56	5280	Company Ltd.	m.kelar	5.00	2020-05-14 13:53:40.106161	OTRS	1	2020-05-19 13:50:54.434662
14	57	5281	Company Ltd.	m.kelar	0.00	2020-05-14 13:53:40.11521	OTRS	1	2020-05-19 13:50:54.434662
15	58	5282	Company Ltd.	m.kelar	5.00	2020-05-14 13:53:41.486921	OTRS	1	2020-05-19 13:52:13.355415
\.


--
-- Name: account_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.account_id_seq', 15, true);


--
-- Name: accounted_time account_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.accounted_time
    ADD CONSTRAINT account_pkey PRIMARY KEY (id);


--
-- Name: TABLE accounted_time; Type: ACL; Schema: public; Owner: postgres
--

GRANT ALL ON TABLE public.accounted_time TO dcpam;


--
-- PostgreSQL database dump complete
--


--
-- PostgreSQL database cluster dump complete
--
