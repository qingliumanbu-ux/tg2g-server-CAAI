/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:
Version:    3.0
Date:		2018-06-21
Description:查询工序产品成本明细
**************************************************/
//框架用头文件
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caaicx41_inq)
//-EP_SYSTEM_HEAD_END

int f_caaicx41_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int fetchRowCount = 0;

	CString		begin_time = " ";
	CString		end_time = " ";
	CString		mat_code = " ";
	//CString		back_code_2 = " ";
	CString		dept_code = " ";
	int rownum = 1;
	CModel tcaai04("TCAAI04");


	CString  sqlstr("");
	CString  sqlstr_where(" ");
	CString  sqlstr_sub(" ");
	CString		flag = "0";
	CString		group_flag = "0";

	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq1(conn);
	CDbCommand cmd_inq_1(conn);

	try
	{
		begin_time = bcls_rec->Tables[0].Rows[0]["BEGIN_TIME"].ToString();
		end_time = bcls_rec->Tables[0].Rows[0]["END_TIME"].ToString();
		dept_code = bcls_rec->Tables[0].Rows[0]["DEPT_CODE"].ToString();
		flag = bcls_rec->Tables[0].Rows[0]["FLAG"].ToString(); //用来判断是单耗还是消耗
		group_flag = bcls_rec->Tables[0].Rows[0]["GROUP_FLAG"].ToString(); //用来判断是否分班组
		tcaai04.MergeFrom(bcls_rec->Tables[0].Rows[0]);

		if (tcaai04["VFREE1"].ToString().Trim() != "")
		{
			sqlstr_where += " AND VFREE1 = @vfree1 ";
		}
		if (tcaai04["SG_SIGN"].ToString().Trim() != "")
		{
			sqlstr_where += " AND SG_SIGN = @sg_sign ";
		}
		if (tcaai04["MAT_THICK"].ToDecimal() != 0)
		{
			sqlstr_where += " AND MAT_THICK = @mat_thick ";
		}
		sqlstr_where += " AND DEPT_CODE=@dept_code AND  STATS_PERIOD>=@begin_time AND STATS_PERIOD<=@end_time ";

		//查消耗项mat_code

		sqlstr = " select  mat_code"
			" from tcaai04 "
			" WHERE	1=1 "
			" and mat_code !=' '"
			" and length(mat_code)<20"
			;
		sqlstr = sqlstr + sqlstr_where;
		sqlstr += " group by mat_code";

		Log::Trace("", __FUNCTION__, "耗用mat_code：sqlstr=[{0}]  ", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("begin_time", begin_time.SubstringNE(0, 8));
		cmd_inq.Parameters.Set("end_time", end_time.SubstringNE(0, 8));
		cmd_inq.Parameters.Set("sg_sign", tcaai04["SG_SIGN"].ToString());
		cmd_inq.Parameters.Set("mat_thick", tcaai04["MAT_THICK"].ToDecimal());
		cmd_inq.Parameters.Set("sub_backlog_code", tcaai04["SUB_BACKLOG_CODE"].ToString());
		cmd_inq.Parameters.Set("vfree1", tcaai04["VFREE1"].ToString());
		cmd_inq.Parameters.Set("dept_code", tcaai04["DEPT_CODE"].ToString());
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			mat_code = cmd_inq.GetString(1);
			sqlstr = " select mat_type FROM TCAAC11"
				" WHERE mat_type in ('B','C','E')"
				" and MAT_CODE = @mat_code"
				;
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("mat_code", mat_code);
			cmd_inq_1.ExecuteReader();
			if (cmd_inq_1.Read())
			{
				if (flag == "1")
				{
					sqlstr_sub += " ,SUM(CASE WHEN MAT_CODE='" + mat_code + "' and OUTPUT!=0 THEN ROUND(COST_YJ/OUTPUT,2) ELSE 0 END)  AS " + mat_code + "_USE";
				}
				else
				{
					sqlstr_sub += " ,SUM(CASE WHEN MAT_CODE='" + mat_code + "' THEN COST_YJ ELSE 0 END)  AS " + mat_code + "_USE";

				}
			}
			else
			{
				if (flag == "1")
				{
					sqlstr_sub += ",SUM(CASE WHEN MAT_CODE= '" + mat_code + "' and OUTPUT!=0 THEN ROUND(WT/OUTPUT,6) ELSE 0 END)  AS " + mat_code + "_USE";
				}
				else
				{
					sqlstr_sub += ",SUM(CASE WHEN MAT_CODE= '" + mat_code + "' THEN WT ELSE 0 END)  AS " + mat_code + "_USE";
				}
			}

			rownum++;
			if (rownum == 100)
			{
				break;
			}
		}
		cmd_inq.Close();
		Log::Trace("", __FUNCTION__, "循环_USE:sql=[{0}]  ", sqlstr_sub);
		if (tcaai04["SUB_BACKLOG_CODE"].ToString().Trim() != "")
		{
			if (group_flag == "1")
			{
				sqlstr = " SELECT T1.VFREE1,T1.SUB_BACKLOG_CODE,T1.PRODUCT_CODE,T1.SG_SIGN,T1.MAT_THICK,T1.EQU_NO,DECODE(WT, 0, 0, ROUND(T1.COST_YJ/T1.WT, 6)) AS PRICE_UNIT_GJ,T1.WT,T1.COST_YJ,T1.STATS_PERIOD AS PROD_DATE,HUISHOU_USE,DONGLI_USE,ZHIZAO_USE,GONGZI_USE,T2.* "
					" FROM "
					" (SELECT SUB_BACKLOG_CODE,VFREE1,PRODUCT_CODE,SG_SIGN,STATS_PERIOD,MAT_THICK,EQU_NO,SUM(WT) AS WT,SUM(COST_YJ) AS COST_YJ FROM TCAAI03 WHERE 1=1 "
					" AND SUB_BACKLOG_CODE=@sub_backlog_code "
					+ sqlstr_where +
					" GROUP BY SUB_BACKLOG_CODE,VFREE1,PRODUCT_CODE,SG_SIGN,MAT_THICK,EQU_NO,STATS_PERIOD ) T1"
					" LEFT JOIN "
					" ( SELECT SUB_BACKLOG_CODE,VFREE1,PRODUCT_CODE,SG_SIGN,MAT_THICK,EQU_NO,STATS_PERIOD  "
					+ sqlstr_sub +
					" FROM TCAAI04 "
					" WHERE 1=1 AND SUB_BACKLOG_CODE=@sub_backlog_code "
					+ sqlstr_where +
					" GROUP BY SUB_BACKLOG_CODE,VFREE1,PRODUCT_CODE,SG_SIGN,MAT_THICK,EQU_NO,STATS_PERIOD ) T2 "
					" ON T1.SUB_BACKLOG_CODE=T2.SUB_BACKLOG_CODE AND T1.VFREE1=T2.VFREE1 AND T1.PRODUCT_CODE=T2.PRODUCT_CODE AND T1.EQU_NO=T2.EQU_NO AND T1.SG_SIGN=T2.SG_SIGN AND T1.MAT_THICK=T2.MAT_THICK AND T1.STATS_PERIOD=T2.STATS_PERIOD  "
					" LEFT JOIN "
					" ( SELECT T04.SUB_BACKLOG_CODE,VFREE1,PRODUCT_CODE,SG_SIGN,STATS_PERIOD,MAT_THICK,EQU_NO,  "
					" SUM(CASE WHEN BACK_CODE_2='D' and @flag ='1' and OUTPUT!=0 THEN ROUND(COST_YJ/OUTPUT,6) when BACK_CODE_2='D' and @flag !='1' then COST_YJ ELSE 0 END) HUISHOU_USE,"
					" SUM(CASE WHEN BACK_CODE_2='A' and @flag ='1' and OUTPUT!=0 THEN ROUND(COST_YJ/OUTPUT,6) when BACK_CODE_2='A' and @flag !='1' then COST_YJ ELSE 0 END) DONGLI_USE,"
					" SUM(CASE WHEN BACK_CODE_2='B' and @flag ='1' and OUTPUT!=0 THEN ROUND(COST_YJ/OUTPUT,6) when BACK_CODE_2='B' and @flag !='1' then COST_YJ ELSE 0 END) GONGZI_USE,"
					" SUM(CASE WHEN BACK_CODE_2='C' and @flag ='1' and OUTPUT!=0 THEN ROUND(COST_YJ/OUTPUT,6) when BACK_CODE_2='C' and @flag !='1' then COST_YJ ELSE 0 END) ZHIZAO_USE"
					" FROM TCAAI04 T04"
					" LEFT JOIN "
					" TCAAC03 T03 ON T04.SUB_BACKLOG_CODE=T03.SUB_BACKLOG_CODE AND T04.MAT_CODE=T03.MAT_CODE "
					" WHERE 1=1 AND T04.SUB_BACKLOG_CODE=@sub_backlog_code "
					+ sqlstr_where +
					" GROUP BY T04.SUB_BACKLOG_CODE,VFREE1,PRODUCT_CODE,SG_SIGN,STATS_PERIOD,MAT_THICK,EQU_NO) T3 "
					" ON T1.SUB_BACKLOG_CODE=T3.SUB_BACKLOG_CODE AND T1.VFREE1=T3.VFREE1 AND T1.PRODUCT_CODE=T3.PRODUCT_CODE AND T1.EQU_NO=T3.EQU_NO AND T1.SG_SIGN=T3.SG_SIGN AND T1.MAT_THICK=T3.MAT_THICK AND T1.STATS_PERIOD=T3.STATS_PERIOD  "
					;
			}
			else
			{
				sqlstr = " SELECT T1.VFREE1,T1.SUB_BACKLOG_CODE,T1.PRODUCT_CODE,T1.SG_SIGN,T1.MAT_THICK,T1.EQU_NO,DECODE(WT, 0, 0, ROUND(T1.COST_YJ/T1.WT, 6)) AS PRICE_UNIT_GJ,T1.WT,T1.COST_YJ,T1.STATS_PERIOD AS PROD_DATE,HUISHOU_USE,DONGLI_USE,ZHIZAO_USE,GONGZI_USE,T2.* "
					" FROM "
					" (SELECT SUB_BACKLOG_CODE,VFREE1,PRODUCT_CODE,SG_SIGN,STATS_PERIOD,MAT_THICK,EQU_NO,SUM(WT) AS WT,SUM(COST_YJ) AS COST_YJ FROM TCAAI03 WHERE 1=1 "
					" AND SUB_BACKLOG_CODE=@sub_backlog_code "
					+ sqlstr_where +
					" GROUP BY SUB_BACKLOG_CODE,VFREE1,PRODUCT_CODE,SG_SIGN,MAT_THICK,EQU_NO,STATS_PERIOD ) T1"
					" LEFT JOIN "
					" ( SELECT SUB_BACKLOG_CODE,VFREE1,PRODUCT_CODE,SG_SIGN,MAT_THICK,EQU_NO,STATS_PERIOD  "
					+ sqlstr_sub +
					" FROM TCAAI04 "
					" WHERE 1=1 AND SUB_BACKLOG_CODE=@sub_backlog_code "
					+ sqlstr_where +
					" GROUP BY SUB_BACKLOG_CODE,VFREE1,PRODUCT_CODE,SG_SIGN,MAT_THICK,EQU_NO,STATS_PERIOD ) T2 "
					" ON T1.SUB_BACKLOG_CODE=T2.SUB_BACKLOG_CODE AND T1.VFREE1=T2.VFREE1 AND T1.PRODUCT_CODE=T2.PRODUCT_CODE AND T1.EQU_NO=T2.EQU_NO AND T1.SG_SIGN=T2.SG_SIGN AND T1.MAT_THICK=T2.MAT_THICK AND T1.STATS_PERIOD=T2.STATS_PERIOD   "
					" LEFT JOIN "
					" ( SELECT T04.SUB_BACKLOG_CODE,VFREE1,PRODUCT_CODE,SG_SIGN,STATS_PERIOD,MAT_THICK,EQU_NO,  "
					" SUM(CASE WHEN BACK_CODE_2='D' and @flag ='1' and OUTPUT!=0 THEN ROUND(COST_YJ/OUTPUT,6) when BACK_CODE_2='D' and @flag !='1' then COST_YJ ELSE 0 END) HUISHOU_USE,"
					" SUM(CASE WHEN BACK_CODE_2='A' and @flag ='1' and OUTPUT!=0 THEN ROUND(COST_YJ/OUTPUT,6) when BACK_CODE_2='A' and @flag !='1' then COST_YJ ELSE 0 END) DONGLI_USE,"
					" SUM(CASE WHEN BACK_CODE_2='B' and @flag ='1' and OUTPUT!=0 THEN ROUND(COST_YJ/OUTPUT,6) when BACK_CODE_2='B' and @flag !='1' then COST_YJ ELSE 0 END) GONGZI_USE,"
					" SUM(CASE WHEN BACK_CODE_2='C' and @flag ='1' and OUTPUT!=0 THEN ROUND(COST_YJ/OUTPUT,6) when BACK_CODE_2='C' and @flag !='1' then COST_YJ ELSE 0 END) ZHIZAO_USE"
					" FROM TCAAI04 T04"
					" LEFT JOIN "
					" TCAAC03 T03 ON T04.SUB_BACKLOG_CODE=T03.SUB_BACKLOG_CODE AND T04.MAT_CODE=T03.MAT_CODE "
					" WHERE 1=1 AND T04.SUB_BACKLOG_CODE=@sub_backlog_code "
					+ sqlstr_where +
					" GROUP BY T04.SUB_BACKLOG_CODE,VFREE1,PRODUCT_CODE,SG_SIGN,STATS_PERIOD,MAT_THICK,EQU_NO) T3 "
					" ON T1.SUB_BACKLOG_CODE=T3.SUB_BACKLOG_CODE AND T1.VFREE1=T3.VFREE1 AND T1.PRODUCT_CODE=T3.PRODUCT_CODE AND T1.EQU_NO=T3.EQU_NO AND T1.SG_SIGN=T3.SG_SIGN AND T1.MAT_THICK=T3.MAT_THICK AND T1.STATS_PERIOD=T3.STATS_PERIOD  "
					;
			}		
				
		}
		else
		{
			if (group_flag == "1")
			{
				
			sqlstr = " SELECT T1.VFREE1,T1.PRODUCT_CODE,T1.SG_SIGN,T1.MAT_THICK,DECODE(WT, 0, 0, ROUND(T1.COST_YJ/T1.WT, 6)) AS PRICE_UNIT_GJ,T1.WT,T1.COST_YJ,T1.STATS_PERIOD AS PROD_DATE,HUISHOU_USE,DONGLI_USE,ZHIZAO_USE,GONGZI_USE,T2.* "
				" FROM "
				" (SELECT VFREE1,PRODUCT_CODE,SG_SIGN,STATS_PERIOD,MAT_THICK,SUM(CASE WHEN SUB_BACKLOG_CODE='C' THEN WT ELSE 0 END) AS WT,SUM(COST_YJ) AS COST_YJ FROM TCAAI03 WHERE 1=1 "   //炼钢产量只取一次，必有连铸工序。
				+ sqlstr_where +
				" GROUP BY VFREE1,PRODUCT_CODE,SG_SIGN,MAT_THICK,STATS_PERIOD ) T1"
				" LEFT JOIN "
				" ( SELECT VFREE1,PRODUCT_CODE,SG_SIGN,MAT_THICK,STATS_PERIOD  "
				+ sqlstr_sub +
				" FROM TCAAI04 "
				" WHERE 1=1 "
				+ sqlstr_where +
				" GROUP BY VFREE1,PRODUCT_CODE,SG_SIGN,MAT_THICK,STATS_PERIOD ) T2 "
				" ON T1.VFREE1=T2.VFREE1 AND T1.PRODUCT_CODE=T2.PRODUCT_CODE AND T1.SG_SIGN=T2.SG_SIGN AND T1.MAT_THICK=T2.MAT_THICK AND T1.STATS_PERIOD=T2.STATS_PERIOD   "
				" LEFT JOIN "
				" ( SELECT VFREE1,PRODUCT_CODE,SG_SIGN,STATS_PERIOD,MAT_THICK,  "
				" SUM(CASE WHEN BACK_CODE_2='D' and @flag ='1' and OUTPUT!=0 THEN ROUND(COST_YJ/OUTPUT,6) when BACK_CODE_2='D' and @flag !='1' then COST_YJ  ELSE 0 END) HUISHOU_USE,"
				" SUM(CASE WHEN BACK_CODE_2='A' and @flag ='1' and OUTPUT!=0 THEN ROUND(COST_YJ/OUTPUT,6) when BACK_CODE_2='A' and @flag !='1' then COST_YJ ELSE 0 END) DONGLI_USE,"
				" SUM(CASE WHEN BACK_CODE_2='B' and @flag ='1' and OUTPUT!=0 THEN ROUND(COST_YJ/OUTPUT,6) when BACK_CODE_2='B' and @flag !='1' then COST_YJ ELSE 0 END) GONGZI_USE,"
				" SUM(CASE WHEN BACK_CODE_2='C' and @flag ='1' and OUTPUT!=0 THEN ROUND(COST_YJ/OUTPUT,6) when BACK_CODE_2='C' and @flag !='1' then COST_YJ ELSE 0 END) ZHIZAO_USE"
				" FROM TCAAI04 T04"
				" LEFT JOIN "
				" TCAAC03 T03 ON T04.SUB_BACKLOG_CODE=T03.SUB_BACKLOG_CODE AND T04.MAT_CODE=T03.MAT_CODE "
				" WHERE 1=1 "
				+ sqlstr_where+
				" GROUP BY VFREE1,PRODUCT_CODE,SG_SIGN,STATS_PERIOD,MAT_THICK) T3 "
				" ON T1.VFREE1=T3.VFREE1 AND T1.PRODUCT_CODE=T3.PRODUCT_CODE AND T1.SG_SIGN=T3.SG_SIGN AND T1.MAT_THICK=T3.MAT_THICK AND T1.STATS_PERIOD=T3.STATS_PERIOD   "
				;
			}
			else
			{
				sqlstr = " SELECT T1.VFREE1,T1.PRODUCT_CODE,T1.SG_SIGN,T1.MAT_THICK,DECODE(WT, 0, 0, ROUND(T1.COST_YJ/T1.WT, 6)) AS PRICE_UNIT_GJ,T1.WT,T1.COST_YJ,T1.STATS_PERIOD AS PROD_DATE,HUISHOU_USE,DONGLI_USE,ZHIZAO_USE,GONGZI_USE,T2.* "
					" FROM "
					" (SELECT VFREE1,PRODUCT_CODE,SG_SIGN,STATS_PERIOD,MAT_THICK,SUM(CASE WHEN SUB_BACKLOG_CODE='C' THEN WT ELSE 0 END) AS WT,SUM(COST_YJ) AS COST_YJ FROM TCAAI03 WHERE 1=1 "   //炼钢产量只取一次，必有连铸工序。
					+ sqlstr_where +
					" GROUP BY VFREE1,PRODUCT_CODE,SG_SIGN,MAT_THICK,STATS_PERIOD ) T1"
					" LEFT JOIN "
					" ( SELECT VFREE1,PRODUCT_CODE,SG_SIGN,MAT_THICK,STATS_PERIOD  "
					+ sqlstr_sub +
					" FROM TCAAI04 "
					" WHERE 1=1 "
					+ sqlstr_where +
					" GROUP BY VFREE1,PRODUCT_CODE,SG_SIGN,MAT_THICK,STATS_PERIOD) T2 "
					" ON T1.VFREE1=T2.VFREE1 AND T1.PRODUCT_CODE=T2.PRODUCT_CODE AND T1.SG_SIGN=T2.SG_SIGN AND T1.MAT_THICK=T2.MAT_THICK AND T1.STATS_PERIOD=T2.STATS_PERIOD  "
					" LEFT JOIN "
					" ( SELECT VFREE1,PRODUCT_CODE,SG_SIGN,STATS_PERIOD,MAT_THICK,  "
					" SUM(CASE WHEN BACK_CODE_2='D' and @flag ='1' and OUTPUT!=0 THEN ROUND(COST_YJ/OUTPUT,6) when BACK_CODE_2='D' and @flag !='1' then COST_YJ  ELSE 0 END) HUISHOU_USE,"
					" SUM(CASE WHEN BACK_CODE_2='A' and @flag ='1' and OUTPUT!=0 THEN ROUND(COST_YJ/OUTPUT,6) when BACK_CODE_2='A' and @flag !='1' then COST_YJ ELSE 0 END) DONGLI_USE,"
					" SUM(CASE WHEN BACK_CODE_2='B' and @flag ='1' and OUTPUT!=0 THEN ROUND(COST_YJ/OUTPUT,6) when BACK_CODE_2='B' and @flag !='1' then COST_YJ ELSE 0 END) GONGZI_USE,"
					" SUM(CASE WHEN BACK_CODE_2='C' and @flag ='1' and OUTPUT!=0 THEN ROUND(COST_YJ/OUTPUT,6) when BACK_CODE_2='C' and @flag !='1' then COST_YJ ELSE 0 END) ZHIZAO_USE"
					" FROM TCAAI04 T04"
					" LEFT JOIN "
					" TCAAC03 T03 ON T04.SUB_BACKLOG_CODE=T03.SUB_BACKLOG_CODE AND T04.MAT_CODE=T03.MAT_CODE "
					" WHERE 1=1 "
					+ sqlstr_where +
					" GROUP BY VFREE1,PRODUCT_CODE,SG_SIGN,STATS_PERIOD,MAT_THICK) T3 "
					" ON T1.VFREE1=T3.VFREE1 AND T1.PRODUCT_CODE=T3.PRODUCT_CODE AND T1.SG_SIGN=T3.SG_SIGN AND T1.MAT_THICK=T3.MAT_THICK AND T1.STATS_PERIOD=T3.STATS_PERIOD  "
					;
			}
		}
		
		Log::Trace("", __FUNCTION__, "sqlstr=[{0}]  ", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("begin_time", begin_time.SubstringNE(0, 8));
		cmd_inq.Parameters.Set("end_time", end_time.SubstringNE(0, 8));
		cmd_inq.Parameters.Set("sg_sign", tcaai04["SG_SIGN"].ToString());
		cmd_inq.Parameters.Set("mat_thick", tcaai04["MAT_THICK"].ToDecimal());
		cmd_inq.Parameters.Set("sub_backlog_code", tcaai04["SUB_BACKLOG_CODE"].ToString());
		cmd_inq.Parameters.Set("vfree1", tcaai04["VFREE1"].ToString());
		cmd_inq.Parameters.Set("dept_code", tcaai04["DEPT_CODE"].ToString());
		cmd_inq.Parameters.Set("flag", flag);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应

		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	s.flag = doFlag;
	bcls_ret->SetSYS(s);
	return doFlag;
}